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
#include "_stream_wrapper.h"
#include "_stream.h"
#include "_mem.h"
#include "_error.h"

static void COMM_CALL __on_deinit(comm_obj_t* obj) {
	_comm_stream_wrapper_t* wrapper = (_comm_stream_wrapper_t*)obj;
	if (wrapper->controller && wrapper->controller->on_deinit) {
		wrapper->controller->on_deinit(obj);
	}
}

static uint32_t COMM_CALL __available_read(const comm_stream_t* stream) {
	return comm_stream_available_read(((_comm_stream_wrapper_t*)stream)->wrapped);
}

static int32_t COMM_CALL __read(comm_stream_t* stream, void* out, uint32_t len) {
	return comm_stream_read(((_comm_stream_wrapper_t*)stream)->wrapped, out, len);
}

static uint32_t COMM_CALL __available_write(const comm_stream_t* stream) {
	return comm_stream_available_write(((_comm_stream_wrapper_t*)stream)->wrapped);
}

static int32_t COMM_CALL __write(comm_stream_t* stream, const void* in, uint32_t len) {
	return comm_stream_write(((_comm_stream_wrapper_t*)stream)->wrapped, in, len);
}

static bool COMM_CALL __flush(comm_stream_t* stream) {
	return comm_stream_flush(((_comm_stream_wrapper_t*)stream)->wrapped);
}

static bool COMM_CALL __close(comm_stream_t* stream) {
	return comm_stream_flush(((_comm_stream_wrapper_t*)stream)->wrapped);
}

static comm_stream_controller_t __streamController = {
	.objController.on_deinit = __on_deinit,

	.available_read  = __available_read,
	.read            = __read,
	.available_write = __available_write,
	.write           = __write,
	.flush           = __flush,
	.close           = __close
};

void _comm_stream_wrapper_init(_comm_stream_wrapper_t* wrapper, comm_stream_t* wrapped, const _comm_stream_wrapper_controller_t* controller, void* data) {
	wrapper->controller = controller;
	wrapper->wrapped    = wrapped;

	_comm_stream_init((comm_stream_t*)wrapper, &__streamController, data);
}

_comm_stream_wrapper_t* _comm_stream_wrapper_new(comm_stream_t* wrapped, const _comm_stream_wrapper_controller_t* controller, void* data) {
	if (!wrapped) {
		errno = COMM_ERROR_INVPARAM;
		return false;
	}

	_comm_stream_wrapper_t* wrapper = _comm_mem_alloc(sizeof(_comm_stream_wrapper_t));

	if (!wrapper)
		goto error;

	_comm_stream_wrapper_init(wrapper, wrapped, controller, data);

	return wrapper;

error:
	if (wrapper)
		_comm_mem_free(wrapper);

	return NULL;
}
