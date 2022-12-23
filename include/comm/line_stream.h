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

typedef comm_stream_t comm_line_stream_t;

#ifdef __cplusplus
extern "C" {
#endif

COMM_PUBLIC comm_line_stream_t* COMM_CALL comm_line_stream_new(comm_stream_t* wrapped, size_t lineMaxLen, bool blockRead);

COMM_PUBLIC bool COMM_CALL comm_line_stream_write(comm_line_stream_t* lineStream, const char* msg);

COMM_PUBLIC const char* COMM_CALL comm_line_stream_read(comm_line_stream_t* lineStream);

#ifdef __cplusplus
} // extern "C"
#endif
