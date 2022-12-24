/*
Copyright (c) 2022 Leandro José Britto de Oliveira

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "_stream_wrapper.h"
#include "_error.h"
#include "_mem.h"

#include <comm/packet_stream.h>

#define __MIN(a,b) (a) <= (b) ? (a) : (b)

typedef struct __packet_stream __packet_stream_t;

struct __packet_stream {
	_comm_stream_wrapper_t wrapper;

	const comm_packet_stream_controller_t* controller;

	int16_t totalRead;
	bool    blockRead;
	uint8_t buffer[256];
};

static void COMM_CALL __on_deinit(comm_obj_t* obj) {
	__packet_stream_t* packetStream = (__packet_stream_t*)obj;

	if (packetStream->controller && packetStream->controller->on_deinit)
		packetStream->controller->on_deinit(obj);
}

COMM_PUBLIC comm_packet_stream_t* COMM_CALL comm_packet_stream_new(comm_stream_t* wrapped, bool blockRead, const comm_packet_stream_controller_t* controller, void* data) {
	static _comm_stream_wrapper_controller_t mWrapperController = {
		.on_deinit = __on_deinit
	};

	if (!wrapped) {
		errno = COMM_ERROR_INVPARAM;
		return NULL;
	}

	__packet_stream_t* packetStream = _comm_mem_alloc(sizeof(__packet_stream_t));

	if (packetStream) {
		packetStream->totalRead = -1;
		packetStream->blockRead = blockRead;
		_comm_stream_wrapper_init((_comm_stream_wrapper_t*)packetStream, wrapped, &mWrapperController, data);
	}

	return (comm_packet_stream_t*)packetStream;
}

COMM_PUBLIC bool COMM_CALL comm_packet_stream_write(comm_packet_stream_t* packetStream, const void* in, uint8_t len) {
	if (!in && len) {
		errno = COMM_ERROR_INVPARAM;
		goto error;
	}

	int32_t written = 0;

	// Header
	do {
		written = comm_stream_write(packetStream, &len, 1);
		if (written < 0) goto error;
	} while (written == 0);

	// Payload
	if (len) {
		uint32_t totalWritten = 0;
		written = 0;
		do {
			written = comm_stream_write(packetStream, in, len);

			if (written < 0) goto error;

			totalWritten += written;
			in           += written;
			len          -= written;
		} while (len > 0);
	}

	return comm_stream_flush(packetStream);

error:
	_COMM_ERROR_SET(COMM_ERROR_IO);
	return false;
}

COMM_PUBLIC uint8_t* COMM_CALL comm_packet_stream_read(comm_packet_stream_t* xPacketStream, uint8_t* lenOut) {
	__packet_stream_t* packetStream = (__packet_stream_t*)xPacketStream;
	int32_t  read;
	uint8_t  len;
	uint8_t* out;

	if (packetStream->blockRead) {
		out = packetStream->buffer;

		// Header
		do {
			read = comm_stream_read(xPacketStream, out, 1);
			if (read < 0) goto error;
		} while (read == 0);

		// Payload
		read = 0;
		len  = out[0];
		out++;

		do {
			read = comm_stream_read(xPacketStream, out, len);

			if (read < 0) goto error;

			out += read;
			len -= read;
		} while (len > 0);

		goto packet_ready;
	} else {
		uint32_t availableRead;
		while (comm_stream_available_read(xPacketStream)) {
			// Header
			if (packetStream->totalRead == -1) {
				out = packetStream->buffer;

				read = comm_stream_read(xPacketStream, out, 1);

				if (read == 0) return NULL;

				if (read < 0) goto error;

				packetStream->totalRead = 0;
			}

			// Payload
			len = packetStream->buffer[0] - packetStream->totalRead;
			out = packetStream->buffer + 1 + packetStream->totalRead;

			availableRead = comm_stream_available_read(xPacketStream);

			if (len > 0 && availableRead > 0) {
				read = comm_stream_read(xPacketStream, out, __MIN(len, availableRead));

				if (read == 0) return NULL;

				if (read < 0) goto error;

				packetStream->totalRead += read;
				len -= read;
			}

			if (len == 0)
				goto packet_ready;
		}

		return NULL;
	}

packet_ready:
	packetStream->totalRead = -1;

	if (lenOut)
		*lenOut = packetStream->buffer[0];

	return packetStream->buffer + 1;

error:
	packetStream->totalRead = -1;
	return NULL;
}
