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
#include "_buffer.h"
#include "_error.h"
#include "_mem.h"

#define __MIN(a,b) (a) <= (b) ? (a) : (b)

static void COMM_CALL __on_deinit(comm_obj_t* obj) {
	_comm_buffer_t* buffer = (_comm_buffer_t*)obj;

	if (buffer->controller && buffer->controller->on_deinit)
		buffer->controller->on_deinit(obj);

	if (!buffer->wrapped && buffer->storage) {
		_comm_mem_free(buffer->storage);
	}
}

static uint32_t COMM_CALL __available_read(const comm_stream_t* stream) {
	_comm_buffer_t* buffer = (_comm_buffer_t*)stream;

	if (buffer->writeCursor < buffer->readCursor) {
		return (buffer->capacity - buffer->readCursor + buffer->writeCursor);
	} else if (buffer->writeCursor > buffer->readCursor) {
		return buffer->writeCursor - buffer->readCursor;
	} else {
		return buffer->lastRead ? 0 : buffer->capacity;
	}
}

static int32_t COMM_CALL __read(comm_stream_t* stream, void* out, uint32_t len) {
	_comm_buffer_t* buffer = (_comm_buffer_t*)stream;

	size_t availableRead = __available_read(stream);
	uint8_t* mOut        = (uint8_t*)out;

	len = __MIN(len, availableRead);

	if (len == 0)
		return 0;

	for (size_t i = 0; i < len; ++i) {
		if (mOut != NULL) {
			mOut[i] = buffer->storage[buffer->readCursor];
		}

		buffer->readCursor++;

		if (buffer->readCursor == buffer->capacity) {
			buffer->readCursor = 0;
		}
	}

	buffer->lastRead = true;

	return len;
}

static uint32_t COMM_CALL __available_write(const comm_stream_t* stream) {
	_comm_buffer_t* buffer = (_comm_buffer_t*)stream;

	if (buffer->writeCursor < buffer->readCursor) {
		return (buffer->readCursor - buffer->writeCursor);
	} else if (buffer->writeCursor > buffer->readCursor) {
		return (buffer->capacity - buffer->writeCursor + buffer->readCursor);
	} else {
		return buffer->lastRead ? buffer->capacity : 0;
	}
}

static int32_t COMM_CALL __write(comm_stream_t* stream, const void* in, uint32_t len) {
	_comm_buffer_t* buffer = (_comm_buffer_t*)stream;

	size_t   availableWrite = __available_write(stream);
	uint8_t* mIn            = (uint8_t*)in;

	len = __MIN(len, availableWrite);

	if (len == 0)
		return 0;

	for (size_t i = 0; i < len; ++i) {
		buffer->storage[buffer->writeCursor] = mIn[i];
		buffer->writeCursor++;

		if (buffer->writeCursor == buffer->capacity)
			buffer->writeCursor = 0;
	}

	buffer->lastRead = false;
	return len;
}

COMM_PUBLIC comm_buffer_t* COMM_CALL comm_buffer_new(size_t capacity, const comm_buffer_controller_t* controller, void* data) {
	static comm_stream_controller_t mController = {
		.objController.on_deinit = __on_deinit,

		.available_read   = __available_read,
		.read             = __read,
		.available_write  = __available_write,
		.write            = __write
	};

	_comm_buffer_t* buffer = _comm_mem_alloc(sizeof(_comm_buffer_t));

	if (!buffer)
		goto error;

	buffer->storage = capacity ? _comm_mem_alloc(capacity) : NULL;

	if (capacity && !buffer->storage)
		goto error;

	buffer->controller = controller;
	buffer->capacity = capacity;
	buffer->readCursor = 0;
	buffer->writeCursor = 0;
	buffer->lastRead = true;
	buffer->wrapped = false;

	_comm_stream_init((comm_stream_t*)buffer, &mController, data);
	return (comm_buffer_t*)buffer;

error:
	if (buffer) {
		if (buffer->storage) {
			_comm_mem_free(buffer->storage);
		}
	}

	return NULL;
}

COMM_PUBLIC bool COMM_CALL comm_buffer_set_storage(comm_buffer_t* xBuffer, uint8_t* storage, size_t capacity, bool empty) {
	if (!storage && capacity) {
		errno = COMM_ERROR_INVPARAM;
		return false;
	}

	_comm_buffer_t* buffer = (_comm_buffer_t*) xBuffer;

	if (!buffer->wrapped && buffer->storage) {
		_comm_mem_free(buffer->storage);
	}

	buffer->wrapped = storage != NULL;
	buffer->storage = storage;
	buffer->capacity = capacity;
	buffer->readCursor = 0;
	buffer->writeCursor = 0;
	buffer->lastRead = empty;

	return buffer;
}

COMM_PUBLIC size_t COMM_CALL comm_buffer_capacity(const comm_buffer_t* xBuffer) {
	return ((_comm_buffer_t*)xBuffer)->capacity;
}

COMM_PUBLIC void COMM_CALL comm_buffer_clear(comm_buffer_t* xBuffer) {
	_comm_buffer_t* buffer = (_comm_buffer_t*) xBuffer;

	buffer->readCursor = 0;
	buffer->writeCursor = 0;
	buffer->lastRead = true;
}
