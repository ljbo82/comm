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
#include "_stream.h"
#include "_mem.h"
#include "_error.h"

COMM_PUBLIC comm_stream_t* COMM_CALL comm_stream_new(const comm_stream_controller_t* controller, void* data) {
	comm_stream_t* stream = _comm_mem_alloc(sizeof(comm_stream_t));

	if (!stream)
		goto error;

	_comm_stream_init(stream, controller, data);
	return stream;
error:
	if (stream)
		_comm_mem_free(stream);

	return NULL;
}

COMM_PUBLIC uint32_t COMM_CALL comm_stream_available_read(const comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;
	if (controller && controller->available_read) {
		return controller->available_read(stream);
	}

	return 0;
}

COMM_PUBLIC int32_t COMM_CALL comm_stream_read(comm_stream_t* stream, void* out, uint32_t len) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->read) {
		int32_t read = controller->read(stream, out, len);
		if (read < 0) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
		}
		return read;
	}

	return 0;
}

COMM_PUBLIC uint32_t COMM_CALL comm_stream_available_write(const comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->available_write) {
		return controller->available_write(stream);
	}

	return 0;
}

COMM_PUBLIC int32_t COMM_CALL comm_stream_write(comm_stream_t* stream, const void* in, uint32_t len) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->write) {
		int32_t read = controller->write(stream, in, len);
		if (read < 0) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
		}
		return read;
	}

	return 0;
}

COMM_PUBLIC bool COMM_CALL comm_stream_flush(comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->flush) {
		if (!controller->flush(stream)) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
			return false;
		}
	}

	return true;
}

COMM_PUBLIC bool COMM_CALL comm_stream_close(comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->close) {
		if (!controller->close(stream)) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
		}
	}

	return true;
}
