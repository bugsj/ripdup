/*
	ripdup - find and remove duplicate files in iso image file

	Copyright(c) 2020 Luo Jie

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this softwareand associated documentation files(the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions :

	The above copyright noticeand this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#pragma once

#include <intrin.h>

inline unsigned int crc32(const void* buf, size_t size)
{
	const unsigned char* p = reinterpret_cast<const unsigned char*>(buf);
	unsigned long long crc64 = 0;
	unsigned int crc = 0;

	while (size) {
		if ((reinterpret_cast<unsigned long long>(p) & 0x7) == 0 && size >= 8) {
			crc64 = crc;
			while (size >= 8) {
				crc64 = _mm_crc32_u64(crc64, *reinterpret_cast<const unsigned long long*>(p));
				p += 8;
				size -= 8;
			}
			crc = static_cast<unsigned int>(crc64);
		}
		else {
			crc = _mm_crc32_u8(crc, *p++);
			--size;
		}
	}

	return crc;
}
