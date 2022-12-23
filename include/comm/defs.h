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

#include "error.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined _WIN32 || defined __CYGWIN__
	#if defined(COMM_BUILD_DLL) && defined(COMM_STATIC_LIB)
		#error COMM_BUILD_DLL and COMM_STATIC_LIB are both defined
	#endif

	#ifdef COMM_BUILD_DLL
		/** @internal */
		#define COMM_PUBLIC __declspec(dllexport)
	#else
		#ifndef COMM_STATIC_LIB
			/** @internal */
			#define COMM_PUBLIC __declspec(dllimport)
		#else
			/** @internal */
			#define COMM_PUBLIC
		#endif
	#endif

	/** @internal */
	#define COMM_CALL __cdecl
#else
	#ifdef COMM_STATIC_LIB
		/** @internal */
		#define COMM_PUBLIC
	#else
		#if __GNUC__ >= 4
			/** @internal */
			#define COMM_PUBLIC __attribute__ ((visibility ("default")))
		#else
			/** @internal */
			#define COMM_PUBLIC
		#endif
	#endif

	/** @internal */
	#define COMM_CALL
#endif
