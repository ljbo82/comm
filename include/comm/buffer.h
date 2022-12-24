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

#include "stream.h"

typedef comm_stream_t         comm_buffer_t;
typedef comm_obj_controller_t comm_buffer_controller_t;

#ifdef __cplusplus
extern "C" {
#endif

COMM_PUBLIC comm_buffer_t* COMM_CALL comm_buffer_new(size_t capacity, const comm_buffer_controller_t* controller, void* data);

COMM_PUBLIC bool COMM_CALL comm_buffer_set_storage(comm_buffer_t* buffer, uint8_t* storage, size_t capacity, bool empty);

COMM_PUBLIC size_t COMM_CALL comm_buffer_capacity(const comm_buffer_t* buffer);

COMM_PUBLIC void COMM_CALL comm_buffer_clear(comm_buffer_t* buffer);

#ifdef __cplusplus
} // extern "C"
#endif
