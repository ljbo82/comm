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
#include "_mem.h"

#include <comm/obj.h>

COMM_PUBLIC comm_obj_t* COMM_CALL comm_obj_new(const comm_obj_controller_t* controller, size_t szObj) {
	comm_obj_t* obj = NULL;

	if (szObj > 0 && szObj < sizeof(comm_obj_t)) {
		errno = COMM_ERROR_INVPARAM;
		goto error;
	}

	obj = _comm_mem_alloc(szObj == 0 ? sizeof(comm_obj_t) : szObj);
	if (!obj) {
		errno = COMM_ERROR_NOMEM;
		goto error;
	}

	obj->controller = controller;

	if (controller && controller->on_init) {
		if (!controller->on_init(obj)) {
			_COMM_ERROR_SET(COMM_ERROR_UNKNOWN);
			goto error;
		}
	}

	return obj;

error:
	if (obj) {
		_comm_mem_free(obj);
	}

	return NULL;
}

COMM_PUBLIC void COMM_CALL comm_obj_del(comm_obj_t* obj) {
	if (obj->controller && obj->controller->on_deinit)
		obj->controller->on_deinit(obj);

	_comm_mem_free(obj);
}
