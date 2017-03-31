#pragma once
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "type_traits.h"

namespace io
{
	template<typename T>
	const uint8_t* xtoa(T value, uint8_t* result, uint8_t base = 10)
	{
		typedef typename stdx::make_unsigned<T>::type UT;
		uint8_t* out = result;
		UT quotient;
		if(stdx::is_signed<T>::value) quotient = stdx::is_negative(value) ? -value : value;
		else quotient = value;
	// check that the base if valid
	//	if (base < 2 || base > 36) { *result = '\0'; return NULL; }
		do {
			const UT q = quotient / base;
			const UT rem = quotient - q * base;
			quotient = q;
			*out++ = (rem < 10 ? '0' : 'a' - 10) + rem;
		} while ( quotient );
		if(stdx::is_signed<T>::value && stdx::is_negative(value)) *out++ = '-';
		*out-- = '\0';
	//reverse string
		uint8_t tmp_char;
		uint8_t* ptr1 = result;
		while (ptr1 < out)
		{
			tmp_char = *out;
			*out-- = *ptr1;
			*ptr1++ = tmp_char;
		}
		return result;
	}
	inline const uint8_t* xtoa(bool val, uint8_t*, uint8_t)
	{
		return val ? "True" : "False";
	}

	#pragma inline=forced
	template<typename T>
	inline static const uint8_t* utoa(T value, uint8_t* result, uint8_t base = 10)
	{
		static_assert(!stdx::is_signed<T>::value, "utoa called with signed arg");
		return xtoa(value, result, base);
	}
	#pragma inline=forced
	template<typename T>
	inline static const uint8_t* itoa(T value, uint8_t* result, uint8_t base = 10)
	{
		static_assert(stdx::is_signed<T>::value, "itoa called with unsigned arg");
		return xtoa(value, result, base);
	}

	#pragma inline=forced
	inline static const uint8_t* utoa8(uint8_t value, uint8_t* result, uint8_t base = 10)
	{
		return utoa(value, result, base);
	}
	#pragma inline=forced
	inline static const uint8_t* utoa16(uint16_t value, uint8_t* result, uint8_t base = 10)
	{
		return utoa(value, result, base);
	}
	#pragma inline=forced
	inline static const uint8_t* utoa32(uint32_t value, uint8_t* result, uint8_t base = 10)
	{
		return utoa(value, result, base);
	}

	#pragma inline=forced
	inline static const uint8_t* itoa8(int8_t value, uint8_t* result, uint8_t base = 10)
	{
		return itoa(value, result, base);
	}
	#pragma inline=forced
	inline static const uint8_t* itoa16(int16_t value, uint8_t* result, uint8_t base = 10)
	{
		return itoa(value, result, base);
	}
	#pragma inline=forced
	inline static const uint8_t* itoa32(int32_t value, uint8_t* result, uint8_t base = 10)
	{
		return itoa(value, result, base);
	}

	uint8_t* InsertDot(uint16_t value, uint8_t position, uint8_t* buf);

}//io

#endif // STRING_UTILS_H

