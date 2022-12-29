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
#include "buffer.h"
#include "../mem.h"
#include "../assert.h"

#include <string.h>
#include <comm/_buffer.h>
#include <inttypes.h>

typedef int32_t (COMM_CALL *__stream_read_f)(comm_stream_t* stream, void* out, uint32_t len);

static __stream_read_f __superReadFn = NULL;

static int32_t COMM_CALL __block_read(comm_stream_t* stream, void* out, uint32_t len) {
	if (comm_stream_available_read(stream) < len) {
		errno = TEST_BUFFER_TIMEOUT_ERROR;
		return -1;
	}

	return __superReadFn(stream, out, len);
}

static void __test_storage_mgmt1() {
	size_t memSize;
	size_t memSize1;
	size_t memSize2;
	uint8_t storage[32];
	uint8_t readBuf[32];

	comm_buffer_t* xBuffer;
	_comm_buffer_t *buffer;

	memSize = mem_size();

	xBuffer = comm_buffer_new(1, NULL, NULL);
	buffer = (_comm_buffer_t*)xBuffer;

	ASSERT(!buffer->wrapped);
	ASSERT(buffer->capacity == 1);
	ASSERT_EQUALS(PRIu32, 0, comm_stream_available_read(xBuffer));
	ASSERT_EQUALS(PRIu32, 1, comm_stream_available_write(xBuffer));

	// Switching to external storage
	memSize1 = mem_size();
	ASSERT(comm_buffer_set_storage(xBuffer, NULL, 0, true));
	memSize2 = mem_size();
	ASSERT (memSize2 < memSize1 && memSize1 > memSize);

	memSize1 = mem_size();
	ASSERT(comm_buffer_set_storage(xBuffer, storage, sizeof(storage), true));
	memSize2 = mem_size();
	ASSERT (memSize2 == memSize1 && memSize1 > memSize);

	comm_obj_del(xBuffer);
	ASSERT(mem_size() == memSize);

	// Buffer with no storage
	xBuffer = comm_buffer_new(0, NULL, NULL);
	buffer = (_comm_buffer_t*)xBuffer;

	ASSERT(buffer->storage == NULL);
	ASSERT(buffer->wrapped == false);
	ASSERT(buffer->capacity == 0);
	ASSERT(comm_stream_available_read(xBuffer) == 0);
	ASSERT(comm_stream_available_write(xBuffer) == 0);
	ASSERT(comm_stream_read(xBuffer, readBuf, sizeof(readBuf)) == 0);
	ASSERT(comm_stream_write(xBuffer, readBuf, sizeof(readBuf)) == 0);

	// Defining external storage
	ASSERT(!comm_buffer_set_storage(xBuffer, NULL, 2, true));
	ASSERT_ERROR(COMM_ERROR_INVPARAM);
	ASSERT(comm_buffer_set_storage(xBuffer, NULL, 0, true));
	ASSERT(comm_buffer_set_storage(xBuffer, storage, sizeof(storage), true));

	ASSERT(buffer->storage == storage);
	ASSERT(buffer->wrapped == true);
	ASSERT(buffer->capacity == sizeof(storage));

	comm_obj_del(xBuffer);
	ASSERT(mem_size() == memSize);
}

static void __test_storage_mgmt2() {
	size_t memSize;
	uint8_t storage[32];
	uint8_t readBuf[32]; // For this function, size must be smaller than, or equal to the size of storage
	comm_buffer_t* buffer;

	memSize = mem_size();

	ASSERT(buffer = comm_buffer_new(0, NULL, NULL));

	ASSERT(comm_buffer_set_storage(buffer, storage, sizeof(storage), true));
	ASSERT(comm_stream_available_read(buffer) == 0);
	ASSERT(comm_stream_available_write(buffer) == sizeof(storage));
	ASSERT(comm_stream_read(buffer, readBuf, sizeof(readBuf)) == 0);
	ASSERT(comm_stream_write(buffer, "hello", strlen("hello")) == strlen("hello"));

	ASSERT(comm_buffer_set_storage(buffer, storage, sizeof(storage), false));
	ASSERT(comm_stream_available_read(buffer) == sizeof(storage));
	ASSERT(comm_stream_available_write(buffer) == 0);
	ASSERT(comm_stream_write(buffer, "hello", strlen("hello")) == 0);
	ASSERT(comm_stream_read(buffer, readBuf, sizeof(readBuf)) == sizeof(readBuf));

	comm_obj_del(buffer);
	ASSERT(mem_size() == memSize);
}

static void __test_read_write() {
	size_t memSize = mem_size();
	uint8_t readBuf[32];

	uint8_t storage[15];
	comm_buffer_t* buffer = comm_buffer_new(0, NULL, NULL);
	comm_buffer_set_storage(buffer, storage, 15, true);

	ASSERT(comm_buffer_capacity(buffer) == 15);
	ASSERT(comm_stream_available_read(buffer) == 0);
	ASSERT(comm_stream_available_write(buffer) == 15);

	ASSERT(comm_stream_write(buffer, "Hello world!", 13) == 13);
	ASSERT(comm_stream_available_read(buffer) == 13);
	ASSERT(comm_stream_available_write(buffer) == 2);

	ASSERT(comm_stream_read(buffer, readBuf, 5) == 5);
	ASSERT(memcmp(readBuf, "Hello", 5) == 0);
	ASSERT(comm_stream_available_read(buffer) == 8);
	ASSERT(comm_stream_available_write(buffer) == 7);

	ASSERT(comm_stream_read(buffer, readBuf, 8) == 8);
	ASSERT(strcmp((const char*)readBuf, " world!") == 0);
	ASSERT(comm_stream_available_read(buffer) == 0);
	ASSERT(comm_stream_available_write(buffer) == 15);

	// Overlapping
	ASSERT(comm_stream_write(buffer, "One more", 8) == 8);

	// No more room
	ASSERT(comm_stream_write(buffer, " time!!!", 9) == 7);

	memset(readBuf, 0, sizeof(readBuf));
	ASSERT(memcmp(storage, "e more time!!On", 15) == 0);
	ASSERT(comm_stream_read(buffer, readBuf, 8) == 8);
	ASSERT(memcmp(readBuf, "One more", 8) == 0);
	ASSERT(comm_stream_read(buffer, readBuf, 256) == 7);
	ASSERT(memcmp(readBuf, " time!!", 7) == 0);

	comm_obj_del(buffer);
	ASSERT(mem_size() == memSize);
}

static void __test_data() {
	size_t memSize = mem_size();

	bool data;
	comm_buffer_t* buffer = comm_buffer_new(12, NULL, &data);
	ASSERT(buffer);
	ASSERT(&data == comm_obj_data(buffer));

	comm_obj_del(buffer);
	ASSERT(mem_size() == memSize);
}

void test_buffer_set_blocking(comm_buffer_t* buffer) {
	static comm_stream_controller_t mBlockingController;
	static bool inited = false;

	comm_stream_controller_t* streamController = (comm_stream_controller_t*)buffer->controller;

	if (streamController == &mBlockingController)
		return;

	if (!inited) {
		__superReadFn = streamController->read;
		memset(&mBlockingController, 0, sizeof(comm_stream_controller_t));
		memcpy(&mBlockingController, streamController, sizeof(comm_stream_controller_t));
		mBlockingController.read = __block_read;
		inited = true;
	}

	buffer->controller = (comm_obj_controller_t*)&mBlockingController;
}

void test_buffer() {
	__test_storage_mgmt1();
	__test_storage_mgmt2();
	__test_read_write();
	__test_data();
}
