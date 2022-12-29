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

typedef struct __obj_data __obj_data_t;
struct __obj_data {
	bool* deleteFlag;
};

static void COMM_CALL __on_deinit(comm_obj_t* xObj) {
	__obj_data_t* data = comm_obj_data(xObj);
	if (data && data->deleteFlag)
		*data->deleteFlag = true;

	mem_free(data);
}

static comm_obj_t* __create_test_obj(bool* deleteFlag) {
	static comm_obj_controller_t mController = {
		.on_deinit = __on_deinit
	};

	__obj_data_t* data;

	if (deleteFlag) {
		data = mem_alloc(sizeof(__obj_data_t));
		ASSERT(data);
		data->deleteFlag = deleteFlag;
		*deleteFlag = false;
	} else {
		data = NULL;
	}

	comm_obj_t* obj = comm_obj_new(&mController, data);
	ASSERT(obj);
	ASSERT(comm_obj_data(obj) == data);

	return obj;
}

static void __test_obj_new() {
	size_t memSize = mem_size();

	comm_obj_t* obj = comm_obj_new(NULL, NULL);

	ASSERT(mem_size() > memSize);
	ASSERT(obj);
	ASSERT(comm_obj_data(obj) == NULL);

	comm_obj_del(obj);

	ASSERT(mem_size() == memSize);
}

static void __test_custom_obj() {
	size_t memSize = mem_size();

	bool deleted;

	comm_obj_t* obj = __create_test_obj(&deleted);

	ASSERT(!deleted);
	ASSERT(mem_size() > memSize);

	comm_obj_del(obj);

	ASSERT(deleted);
	ASSERT(mem_size() == memSize);
}

void test_obj() {
	__test_obj_new();
	__test_custom_obj();
}
