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
#include "_mem.h"

#if defined __has_include
	#if __has_include (<comm_config.h>)
		#include <comm_config.h>
	#endif
#endif

#ifndef _COMM_MEM_HEADER
	#define _COMM_MEM_HEADER <stdlib.h>
#endif

#ifndef _COMM_MEM_ALLOC_FN
	#define _COMM_MEM_ALLOC_FN malloc
#endif

#ifndef _COMM_MEM_FREE_FN
	#define _COMM_MEM_FREE_FN free
#endif

#include _COMM_MEM_HEADER

COMM_PUBLIC void* COMM_CALL _comm_mem_alloc(size_t size) {
	void* ptr = _COMM_MEM_ALLOC_FN(size);
	if (!ptr)
		errno = COMM_ERROR_NOMEM;

	return ptr;
}

COMM_PUBLIC void COMM_CALL _comm_mem_free(void* ptr) {
	_COMM_MEM_FREE_FN(ptr);
}
