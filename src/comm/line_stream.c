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
#include "_error.h"
#include "_stream.h"
#include "_mem.h"

#include <comm/line_stream.h>

#define __LINE_LIMIT     (UINT16_MAX - 1)
#define __LINE_DELIMITER '\n'

COMM_OBJ_DECLARE_BEGIN(__line_stream, _comm_stream_wrapper);
	uint16_t totalRead;
	bool     blockRead;
	size_t   lineMaxLen;
	uint8_t* buffer;
COMM_OBJ_DECLARE_END();

static void COMM_CALL __on_deinit(comm_obj_t* obj) {
	__line_stream_t* lineStream = (__line_stream_t*)obj;
	_comm_mem_free(lineStream->buffer);
}

COMM_PUBLIC comm_line_stream_t* COMM_CALL comm_line_stream_new(comm_stream_t* wrapped, size_t lineMaxLen, bool blockRead) {
	static comm_obj_controller_t mWrapperController = {
		.on_deinit = __on_deinit
	};

	if (lineMaxLen > __LINE_LIMIT || lineMaxLen == 0) {
		errno = COMM_ERROR_INVPARAM;
		return NULL;
	}

	__line_stream_t* lineStream = (__line_stream_t*)_comm_stream_wrap(&mWrapperController, wrapped, sizeof(__line_stream_t));

	if (!lineStream) goto error;

	lineStream->totalRead = 0;
	lineStream->blockRead = blockRead;
	lineStream->lineMaxLen = lineMaxLen;
	lineStream->buffer = _comm_mem_alloc(lineMaxLen + 1);

	return (comm_line_stream_t*)lineStream;
error:
	if (lineStream) {
		if (lineStream->buffer) {
			_comm_mem_free(lineStream->buffer);
		}
		_comm_mem_free (lineStream);
	}

	return NULL;
}

COMM_PUBLIC bool COMM_CALL comm_line_stream_write(comm_line_stream_t* xLineStream, const char* msg) {
	if (!msg)
		msg = "";

	bool endsWithNewLine = false;
	int32_t written;

	while(*msg) {
		endsWithNewLine = (*msg == __LINE_DELIMITER);
		written = comm_stream_write(xLineStream, msg, 1);
		if (written < 0) goto error;

		if (written > 0) {
			msg++;
		}
	}

	uint8_t newLine = __LINE_DELIMITER;
	if (!endsWithNewLine) {
		do {
			written = comm_stream_write(xLineStream, &newLine, 1);
			if (written < 0) goto error;
		} while (written == 0);
	}

	return comm_stream_flush(xLineStream);

error:
	return false;
}

COMM_PUBLIC const char* COMM_CALL comm_line_stream_read(comm_line_stream_t* xLineStream) {
	__line_stream_t* lineStream = (__line_stream_t*)xLineStream;

	int32_t read;
	uint8_t* out;

	if (lineStream->blockRead) {
		lineStream->totalRead = 0;
		out = lineStream->buffer;

		while(true) {
			read = comm_stream_read(xLineStream, out, 1);

			if (read < 0) goto error;
			if (read == 0) continue;

			if (*out == __LINE_DELIMITER) {
				*out = '\0';
				return (const char*)lineStream->buffer;
			} else {
				lineStream->totalRead++;
				if (lineStream->totalRead <= lineStream->lineMaxLen) {
					out++;
				} else {
					lineStream->totalRead = lineStream->lineMaxLen;
				}
			}
		}
	} else {
		while(comm_stream_available_read(xLineStream)) {
			out = lineStream->buffer + lineStream->totalRead;
			read = comm_stream_read(xLineStream, out, 1);

			if (read < 0) goto error;
			if (read == 0) return NULL;

			if (*out == __LINE_DELIMITER) {
				*out = '\0';
				lineStream->totalRead = 0;
				return (const char*)lineStream->buffer;
			}

			lineStream->totalRead++;

			if (lineStream->totalRead > lineStream->lineMaxLen) {
				lineStream->totalRead = lineStream->lineMaxLen;
			}
		}

		return NULL;
	}

error:
	return NULL;
}
