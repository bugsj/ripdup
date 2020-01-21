#pragma once

#include <intrin.h>

inline unsigned int crc32(const void* buf, size_t size)
{
	const unsigned char* p = reinterpret_cast<const unsigned char*>(buf);
	unsigned long long crc = 0;

	while (size) {
		if ((reinterpret_cast<unsigned long long>(p) & 0x7) == 0 && size >= 8) {
			while (size >= 8) {
				crc = _mm_crc32_u64(crc, *reinterpret_cast<const unsigned long long*>(p));
				p += 8;
				size -= 8;
			}
		}
		else {
			crc = _mm_crc32_u8(static_cast<unsigned int>(crc), *p++);
			--size;
		}
	}

	return static_cast<unsigned int>(crc);
}
