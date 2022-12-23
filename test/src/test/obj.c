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
#include "obj.h"
#include "../mem.h"
#include "../assert.h"

COMM_OBJ_DECLARE_BEGIN(__obj, comm_obj);
	size_t memSize;
COMM_OBJ_DECLARE_END();

static bool __inited = false;
static bool __deinited = false;

static bool COMM_CALL __on_init(comm_obj_t* xObj) {
	__inited = true;
	__deinited = false;

	__obj_t* obj = (__obj_t*)xObj;
	obj->memSize = mem_size();

	return true;
}

static void COMM_CALL __on_deinit(comm_obj_t* xObj) {
	__deinited = true;
}

static void __test_obj_new() {
	size_t memSize = mem_size();
	ASSERT(comm_obj_new(NULL, 1) == NULL);
	ASSERT_ERROR(COMM_ERROR_INVPARAM);

	comm_obj_t* commObj;
	ASSERT(commObj = comm_obj_new(NULL, 0));
	comm_obj_del(commObj);
	ASSERT(mem_size() == memSize);
}

static void __test_custom_obj() {
	comm_obj_controller_t controller = {
		.on_init   = __on_init,
		.on_deinit = __on_deinit
	};

	size_t memSize = mem_size();

	comm_obj_t* obj;

	ASSERT(!__inited);
	ASSERT(!__deinited);

	ASSERT(obj = comm_obj_new(&controller, sizeof(__obj_t)));
	ASSERT(((__obj_t*)obj)->memSize == mem_size());

	ASSERT(__inited);
	ASSERT(!__deinited);

	comm_obj_del(obj);

	ASSERT(__inited);
	ASSERT(__deinited);

	ASSERT(mem_size() == memSize);
}

void test_obj() {
	__test_obj_new();
	__test_custom_obj();
}
