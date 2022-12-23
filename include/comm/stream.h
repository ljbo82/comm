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
#pragma once

#include "obj.h"

typedef comm_obj_t comm_stream_t;

COMM_OBJ_DECLARE_BEGIN(comm_stream_controller, comm_obj_controller);
	uint32_t (COMM_CALL *available_read)(const comm_stream_t* stream);
	int32_t (COMM_CALL *read)(comm_stream_t* stream, void* out, uint32_t len);

	uint32_t (COMM_CALL *available_write)(const comm_stream_t* stream);
	int32_t (COMM_CALL *write)(comm_stream_t* stream, const void* in, uint32_t len);
	bool (COMM_CALL *flush)(comm_stream_t* stream);

	bool (COMM_CALL *close)(comm_stream_t* stream);
COMM_OBJ_DECLARE_END();

#ifdef __cplusplus
extern "C" {
#endif

COMM_PUBLIC comm_stream_t* COMM_CALL comm_stream_new(const comm_stream_controller_t* controller, size_t szStream);

COMM_PUBLIC uint32_t COMM_CALL comm_stream_available_read(const comm_stream_t* stream);

COMM_PUBLIC int32_t COMM_CALL comm_stream_read(comm_stream_t* stream, void* out, uint32_t len);

COMM_PUBLIC uint32_t COMM_CALL comm_stream_available_write(const comm_stream_t* stream);

COMM_PUBLIC int32_t COMM_CALL comm_stream_write(comm_stream_t* stream, const void* in, uint32_t len);

COMM_PUBLIC bool COMM_CALL comm_stream_flush(comm_stream_t* stream);

COMM_PUBLIC bool COMM_CALL comm_stream_close(comm_stream_t* stream);

#ifdef __cplusplus
} // extern "C"
#endif
