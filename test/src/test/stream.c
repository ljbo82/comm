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
#include "stream.h"
#include "../mem.h"
#include "../assert.h"

#include <inttypes.h>

static void __test_data() {
	size_t memSize = mem_size();

	bool data;

	comm_stream_t* stream = comm_stream_new(NULL, &data);
	ASSERT(stream);
	ASSERT(&data == comm_obj_data(stream));

	comm_obj_del(stream);

	ASSERT(mem_size() == memSize);
}

static void __test_void_stream() {
	size_t memSize = mem_size();
	comm_stream_t* stream = comm_stream_new(NULL, 0);
	ASSERT (stream);
	ASSERT_EQUALS(PRIu32, 0, comm_stream_available_read(stream));
	ASSERT_EQUALS(PRIu32, 0, comm_stream_available_write(stream));

	uint8_t buffer[32];
	ASSERT_EQUALS(PRIu32, 0, comm_stream_read(stream, buffer, sizeof(buffer)));
	ASSERT_EQUALS(PRIu32, 0, comm_stream_write(stream, buffer, sizeof(buffer)));
	ASSERT(comm_stream_flush(stream));
	ASSERT(comm_stream_close(stream));

	ASSERT_ERROR(COMM_ERROR_NO_ERROR);
	comm_obj_del(stream);

	ASSERT(mem_size() == memSize);
}

void test_stream() {
	__test_void_stream();
	__test_data();
}
