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
#include "mem.h"
#include "../mem.h"
#include "../assert.h"

#include <comm/_mem.h>

static void __test_mem() {
	void* ptr;
	ASSERT(mem_size() == 0);
	ASSERT(ptr = mem_alloc(12));
	ASSERT(mem_size() == 12);
	mem_free(ptr);
	ASSERT(mem_size() == 0);
}

static void __test_override() {
	void* ptr;
	ASSERT(mem_size() == 0);
	ASSERT(ptr = _comm_mem_alloc(12));
	ASSERT(mem_size() == 12);
	_comm_mem_free(ptr);
	ASSERT(mem_size() == 0);
}

void test_mem() {
	__test_mem();
	__test_override();
}
