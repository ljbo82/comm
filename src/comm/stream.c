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
#include "_stream.h"

static void COMM_CALL __on_deinit(comm_obj_t* obj) {
	_comm_stream_wrapper_t* wrapper = (_comm_stream_wrapper_t*)obj;
	if (wrapper->wrapperController && wrapper->wrapperController->on_deinit) {
		wrapper->wrapperController->on_deinit(obj);
	}
}

static uint32_t COMM_CALL __available_read(const comm_stream_t* stream) {
	return comm_stream_available_read(((_comm_stream_wrapper_t*)stream)->wrappedStream);
}

static int32_t COMM_CALL __read(comm_stream_t* stream, void* out, uint32_t len) {
	return comm_stream_read(((_comm_stream_wrapper_t*)stream)->wrappedStream, out, len);
}

static uint32_t COMM_CALL __available_write(const comm_stream_t* stream) {
	return comm_stream_available_write(((_comm_stream_wrapper_t*)stream)->wrappedStream);
}

static int32_t COMM_CALL __write(comm_stream_t* stream, const void* in, uint32_t len) {
	return comm_stream_write(((_comm_stream_wrapper_t*)stream)->wrappedStream, in, len);
}

static bool COMM_CALL __flush(comm_stream_t* stream) {
	return comm_stream_flush(((_comm_stream_wrapper_t*)stream)->wrappedStream);
}

static bool COMM_CALL __close(comm_stream_t* stream) {
	return comm_stream_flush(((_comm_stream_wrapper_t*)stream)->wrappedStream);
}

_comm_stream_wrapper_t* _comm_stream_wrap(const comm_obj_controller_t* wrapperController, comm_stream_t* wrappedStream, size_t szWrapper) {
	if ((szWrapper > 0 && szWrapper < sizeof(_comm_stream_wrapper_t)) || !wrappedStream) {
		errno = COMM_ERROR_INVPARAM;
		return NULL;
	}

	static comm_stream_controller_t mStreamController = {
		.comm_obj_controller.on_deinit = __on_deinit,

		.available_read  = __available_read,
		.read            = __read,
		.available_write = __available_write,
		.write           = __write,
		.flush           = __flush,
		.close           = __close
	};

	_comm_stream_wrapper_t* wrapper = (_comm_stream_wrapper_t*)comm_stream_new(&mStreamController, szWrapper);

	if (!wrapper)
		goto error;

	wrapper->wrapperController = wrapperController;
	wrapper->wrappedStream = wrappedStream;

	// wrapperController's on_init() must be called manually because
	// it may use delayed initialized values
	if (wrapperController && wrapperController->on_init) {
		if (!wrapperController->on_init((comm_obj_t*)wrapper)) {
			_COMM_ERROR_SET(COMM_ERROR_UNKNOWN);
			goto error;
		}
	}

	return wrapper;

error:
	if (wrapper) {
		// Assigning wrapperController to NULL prevents
		// wrapperController->on_delete() from being called during
		// the call to comm_obj_del() (NOTE: wrapper was not correctly
		// initialized at this point)
		wrapper->wrapperController = NULL;
		comm_obj_del((comm_obj_t*)wrapper);
		wrapper = NULL;
	}

	return wrapper;
}

COMM_PUBLIC comm_stream_t* COMM_CALL comm_stream_new(const comm_stream_controller_t* controller, size_t szStream) {
	return comm_obj_new((comm_obj_controller_t*)controller, szStream);
}

COMM_PUBLIC uint32_t COMM_CALL comm_stream_available_read(const comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;
	if (controller && controller->available_read) {
		return controller->available_read(stream);
	}

	return 0;
}

COMM_PUBLIC int32_t COMM_CALL comm_stream_read(comm_stream_t* stream, void* out, uint32_t len) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->read) {
		int32_t read = controller->read(stream, out, len);
		if (read < 0) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
		}
		return read;
	}

	return 0;
}

COMM_PUBLIC uint32_t COMM_CALL comm_stream_available_write(const comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->available_write) {
		return controller->available_write(stream);
	}

	return 0;
}

COMM_PUBLIC int32_t COMM_CALL comm_stream_write(comm_stream_t* stream, const void* in, uint32_t len) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->write) {
		int32_t read = controller->write(stream, in, len);
		if (read < 0) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
		}
		return read;
	}

	return 0;
}

COMM_PUBLIC bool COMM_CALL comm_stream_flush(comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->flush) {
		if (!controller->flush(stream)) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
			return false;
		}
	}

	return true;
}

COMM_PUBLIC bool COMM_CALL comm_stream_close(comm_stream_t* stream) {
	const comm_stream_controller_t* controller = (const comm_stream_controller_t*)stream->controller;

	if (controller && controller->close) {
		if (!controller->close(stream)) {
			_COMM_ERROR_SET(COMM_ERROR_IO);
		}
	}

	return true;
}
