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
#include "packet_stream.h"
#include "buffer.h"
#include "../mem.h"
#include "../assert.h"
#include <string.h>
#include <stdarg.h>

static void __wrapping_test() {
	ASSERT(!comm_packet_stream_new(NULL, false, NULL, NULL));
	ASSERT_ERROR(COMM_ERROR_INVPARAM);
}

static void __create_test_packet(comm_buffer_t* buffer, uint8_t len) {
	ASSERT(comm_stream_write(buffer, &len, 1) == 1); // Header
	for (uint8_t i = 0; i < len; i++) {
		// Payload
		ASSERT(comm_stream_write(buffer, (uint8_t*)&i, 1) == 1);
	}
}

static void __write_packet_chunk(comm_buffer_t* buffer, uint8_t len, ...) {
	va_list bytes;
	va_start(bytes, len);
	for (uint8_t i = 0; i < len; i++) {
		uint8_t b = (uint8_t)va_arg(bytes, int);
		ASSERT(comm_stream_write(buffer, &b, 1));
	}
	va_end(bytes);
}

static void __assert_test_packet(uint8_t* payload, uint8_t len) {
	for (uint8_t i = 0; i < len ; i++) {
		ASSERT(payload[i] == i);
	}
}

static void __blocking_buffer_read_test() {
	uint8_t* packet;

	size_t memSize = mem_size();

	comm_buffer_t* buffer = comm_buffer_new(1024, NULL, NULL);
	ASSERT(buffer);

	test_buffer_set_blocking(buffer);

	comm_packet_stream_t* packetStream = comm_packet_stream_new(buffer, true, NULL, NULL);
	ASSERT(packetStream);

	uint8_t len = 0;
	ASSERT(!comm_packet_stream_read(packetStream, &len));
	ASSERT_ERROR(TEST_BUFFER_TIMEOUT_ERROR);

	// Zero size packet
	len = 123;
	__create_test_packet(buffer, 0);
	ASSERT(comm_stream_available_read(buffer) == 1);
	ASSERT(comm_packet_stream_read(packetStream, &len));
	ASSERT(comm_stream_available_read(buffer) == 0);
	ASSERT(len == 0);

	// Packet with size 1
	len = 123;
	uint8_t b = 1;
	ASSERT(comm_packet_stream_write(packetStream, &b, 1));
	ASSERT(comm_stream_available_read(buffer) == 2);
	ASSERT(comm_packet_stream_read(packetStream, &len));
	ASSERT(comm_stream_available_read(buffer) == 0);
	ASSERT(len == 1);

	// Packet with size 10
	__create_test_packet(buffer, 10);
	len = 0;
	ASSERT(packet = comm_packet_stream_read(packetStream, &len));
	ASSERT(len == 10);
	__assert_test_packet(packet, len);

	ASSERT(!comm_packet_stream_read(packetStream, &len));
	ASSERT_ERROR(TEST_BUFFER_TIMEOUT_ERROR);

	// Tow packets together (first with size 5 and the second with size 18)
	comm_buffer_clear(buffer);
	__create_test_packet(buffer, 5);
	__create_test_packet(buffer, 18);
	len = 0;
	ASSERT(packet = comm_packet_stream_read(packetStream, &len));
	ASSERT(len == 5);
	__assert_test_packet(packet, len);
	ASSERT(packet = comm_packet_stream_read(packetStream, &len));
	ASSERT(len == 18);
	__assert_test_packet(packet, len);

	ASSERT(!comm_packet_stream_read(packetStream, &len));
	ASSERT_ERROR(TEST_BUFFER_TIMEOUT_ERROR);

	comm_obj_del(buffer);
	comm_obj_del(packetStream);
	ASSERT(mem_size() == memSize);
}

static void __non_blocking_buffer_read_test() {
	size_t memSize = mem_size();

	comm_buffer_t* buffer = comm_buffer_new(1024, NULL, NULL);
	test_buffer_set_blocking(buffer);

	comm_packet_stream_t* packetStream = comm_packet_stream_new(buffer, false, NULL, NULL);
	ASSERT(packetStream);

	ASSERT(!comm_packet_stream_read(packetStream, NULL));
	ASSERT(errno == 0);

	uint8_t len;
	uint8_t* packet;

	// Zero-size packet
	len = 123;
	__create_test_packet(buffer, 0);
	ASSERT(comm_packet_stream_read(packetStream, &len));
	ASSERT(len == 0);

	// Tow packets together (3, 5)
	comm_buffer_clear(buffer);
	__write_packet_chunk(buffer, 1, 3); // Packet1 header
	__write_packet_chunk(buffer, 2, 0, 1); // { 0, 1 } (Incomplete packet1)

	ASSERT(!comm_packet_stream_read(packetStream, &len));
	ASSERT(errno == 0);

	__write_packet_chunk(buffer, 1, 2); // { 0, 1, 2 } (packet1 is complete)
	__write_packet_chunk(buffer, 1, 5); // Packet2 header
	__write_packet_chunk(buffer, 5, 0, 1, 2, 3, 4);  // {0, 1, 2}, {0, 1, 2, 3, 4} (packet2 is complete)

	len = 0;
	ASSERT(packet = comm_packet_stream_read(packetStream, &len));
	ASSERT(len == 3);
	__assert_test_packet(packet, len);
	ASSERT(packet = comm_packet_stream_read(packetStream, &len));
	ASSERT(len == 5);
	__assert_test_packet(packet, 5);

	comm_obj_del(buffer);
	comm_obj_del(packetStream);
	ASSERT(mem_size() == memSize);
}

static void __write_test() {
	size_t memSize = mem_size();

	uint8_t storage[36];

	comm_buffer_t* buffer = comm_buffer_new(0, NULL, NULL);
	ASSERT(buffer);
	comm_buffer_set_storage(buffer, storage, sizeof(storage), true);

	comm_packet_stream_t* packetStream = comm_packet_stream_new(buffer, false, NULL, NULL);
	ASSERT(packetStream);

	ASSERT(!comm_packet_stream_write(packetStream, NULL, 12));
	ASSERT_ERROR(COMM_ERROR_INVPARAM);

	// Empty packet
	memset(storage, 1, sizeof(storage));
	comm_packet_stream_write(packetStream, NULL, 0);
	ASSERT(storage[0] == 0);
	comm_buffer_clear(buffer);

	// 6-byte packet
	const char* packet = "PACKET";
	memset(storage, 0, sizeof(storage));
	comm_packet_stream_write(packetStream, packet, strlen(packet));
	ASSERT(storage[0] == strlen(packet));
	ASSERT(memcmp(storage + 1, packet, storage[0]) == 0);
	comm_buffer_clear(buffer);

	comm_obj_del(buffer);
	comm_obj_del(packetStream);
	ASSERT(mem_size() == memSize);
}

static void __test_data() {
	size_t memSize = mem_size();

	bool data;

	comm_stream_t* stream = comm_stream_new(NULL, NULL);
	ASSERT(stream);

	comm_packet_stream_t* packetStream = comm_packet_stream_new(stream, false, NULL, &data);
	ASSERT(packetStream);
	ASSERT(&data == comm_obj_data(packetStream));

	comm_obj_del(packetStream);
	comm_obj_del(stream);

	ASSERT(mem_size() == memSize);
}

void test_packet_stream() {
	__wrapping_test();
	__blocking_buffer_read_test();
	__non_blocking_buffer_read_test();
	__write_test();
	__test_data();
}
