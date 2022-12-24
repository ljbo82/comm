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
#include "line_stream.h"
#include "buffer.h"
#include "../mem.h"
#include "../assert.h"
#include <string.h>

static void __wrapping_test() {
	ASSERT(!comm_line_stream_new(NULL, 1023, false, NULL, NULL));
	ASSERT_ERROR(COMM_ERROR_INVPARAM);
}

static void __blocking_buffer_read_test() {
	size_t memSize = mem_size();

	comm_buffer_t* buffer = comm_buffer_new(1024, NULL, NULL);
	test_buffer_set_blocking(buffer);

	comm_line_stream_t* lineStream = comm_line_stream_new(buffer, 1023, true, NULL, NULL);
	ASSERT(lineStream);

	ASSERT(!comm_line_stream_read(lineStream));
	ASSERT_ERROR(TEST_BUFFER_TIMEOUT_ERROR);

	const char* lines = "line1\nline2\n";
	comm_stream_write(buffer, lines, strlen(lines));
	const char* line = comm_line_stream_read(lineStream);
	ASSERT(line);
	ASSERT_STR_EQUALS("line1", line);
	ASSERT(comm_stream_available_read(buffer) > 0);
	ASSERT(errno == 0);

	line = comm_line_stream_read(lineStream);
	ASSERT(line);
	ASSERT_STR_EQUALS("line2", line);
	ASSERT(comm_stream_available_read(buffer) == 0);
	ASSERT(errno == 0);

	line = comm_line_stream_read(lineStream);
	ASSERT(!line);
	ASSERT_ERROR(TEST_BUFFER_TIMEOUT_ERROR);

	comm_obj_del(buffer);
	comm_obj_del(lineStream);
	ASSERT(mem_size() == memSize);
}

static void __non_blocking_buffer_read_test() {
	size_t memSize = mem_size();

	comm_buffer_t* buffer = comm_buffer_new(1024, NULL, NULL);
	test_buffer_set_blocking(buffer);

	comm_line_stream_t* lineStream = comm_line_stream_new(buffer, 1023, false, NULL, NULL);
	ASSERT(lineStream);

	ASSERT(!comm_line_stream_read(lineStream));
	ASSERT(errno == 0);

	const char* lines;
	const char* line;

	lines = "lin";
	comm_stream_write(buffer, lines, strlen(lines));

	line = comm_line_stream_read(lineStream);
	ASSERT(!line);
	ASSERT(errno == 0);

	lines = "e1\nLI";
	comm_stream_write(buffer, lines, strlen(lines));

	line = comm_line_stream_read(lineStream);
	ASSERT(line);
	ASSERT_STR_EQUALS("line1", line);

	line = comm_line_stream_read(lineStream);
	ASSERT(!line);
	ASSERT(errno == 0);

	lines = "NE2\n";
	comm_stream_write(buffer, lines, strlen(lines));

	line = comm_line_stream_read(lineStream);
	ASSERT(line);
	ASSERT_STR_EQUALS("LINE2", line);
	ASSERT(comm_stream_available_read(buffer) == 0);

	line = comm_line_stream_read(lineStream);
	ASSERT(!line);
	ASSERT(errno == 0);

	comm_obj_del(buffer);
	comm_obj_del(lineStream);
	ASSERT(mem_size() == memSize);
}

static void __write_test() {
	size_t memSize = mem_size();

	uint8_t storage[16];

	comm_buffer_t* buffer = comm_buffer_new(0, NULL, NULL);
	ASSERT(buffer);
	comm_buffer_set_storage(buffer, storage, sizeof(storage), true);

	comm_line_stream_t* lineStream = comm_line_stream_new(buffer, 1023, false, NULL, NULL);
	ASSERT(lineStream);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, NULL);
	ASSERT(memcmp(storage, "\n", 1) == 0);
	comm_buffer_clear(buffer);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, "LINE");
	ASSERT(memcmp(storage, "LINE\n", 5) == 0);
	comm_buffer_clear(buffer);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, "LINE\n");
	ASSERT(memcmp(storage, "LINE\n", 5) == 0);
	comm_buffer_clear(buffer);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, "LINE1\nline2");
	ASSERT(memcmp(storage, "LINE1\nline2\n", 12) == 0);
	comm_buffer_clear(buffer);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, "LINE1\nline2\n");
	ASSERT(memcmp(storage, "LINE1\nline2\n", 12) == 0);
	comm_buffer_clear(buffer);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, "");
	ASSERT(memcmp(storage, "\n", 1) == 0);
	comm_buffer_clear(buffer);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, "\n");
	ASSERT(memcmp(storage, "\n", 1) == 0);
	comm_buffer_clear(buffer);

	memset(storage, 0, sizeof(storage));
	comm_line_stream_write(lineStream, "\n\n");
	ASSERT(memcmp(storage, "\n\n", 2) == 0);
	comm_buffer_clear(buffer);

	comm_obj_del(buffer);
	comm_obj_del(lineStream);
	ASSERT(mem_size() == memSize);
}

static void __test_data() {
	size_t memSize = mem_size();

	bool data;

	comm_stream_t* stream = comm_stream_new(NULL, NULL);
	ASSERT(stream);

	comm_line_stream_t* lineStream = comm_line_stream_new(stream, 12, false, NULL, &data);
	ASSERT(lineStream);
	ASSERT(&data == comm_obj_data(lineStream));

	comm_obj_del(lineStream);
	comm_obj_del(stream);

	ASSERT(mem_size() == memSize);
}

void test_line_stream() {
	__wrapping_test();
	__blocking_buffer_read_test();
	__non_blocking_buffer_read_test();
	__write_test();
	__test_data();
}
