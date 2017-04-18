/*
 * Copyright (c) 2017 Dmytro Shestakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include "stdint.h"
#include <stddef.h>

namespace stdx
{
	template<uint32_t val>
	struct Int2Type
	{
		enum {value = val};
	};

	template<bool b, typename T1, typename T2>
	struct conditional
	{
		typedef T1 type;
	};

	template<typename T1, typename T2>
	struct conditional<false, T1, T2>
	{
		typedef T2 type;
	};

	template<size_t sizeBits>
	struct SelectSize
	{
		static const bool LessOrEq8 = sizeBits <= 8;
		static const bool LessOrEq16 = sizeBits <= 16;
		static const bool LessOrEq32 = sizeBits <= 32;

		typedef typename conditional<
				LessOrEq8,
				uint8_t,
				typename conditional<LessOrEq16, uint16_t, uint32_t>::type>
				::type type;
	};

	template<unsigned size>
	struct SelectSizeForLength
	{
		static const bool LessOrEq8 = size <= 0xff;
		static const bool LessOrEq16 = size <= 0xffff;

		typedef typename conditional<
				LessOrEq8,
				uint8_t,
				typename conditional<LessOrEq16, uint16_t, uint32_t>::type>
				::type type;
	};

	template<typename T>
	struct make_unsigned
	{
		typedef T type;
	};
	template<> struct make_unsigned<int32_t>
	{
	  typedef uint32_t type;
	};
	template<> struct make_unsigned<int16_t>
	{
	  typedef uint16_t type;
	};
	template<> struct make_unsigned<int8_t>
	{
	  typedef uint8_t type;
	};

	template<typename T>
	struct is_signed
	{
		static const bool value = false;
	};
	template<> struct is_signed<int8_t> { static const bool value = true; };
	template<> struct is_signed<int16_t> { static const bool value = true; };
	template<> struct is_signed<int32_t> { static const bool value = true; };

	template<typename T, typename U>
	struct is_same
	{
		static const bool value = false;
	};
	template<typename T>
	struct is_same<T, T>
	{
		static const bool value = true;
	};

	template<bool b, typename T = void>
	struct enable_if {};
	template<typename T>
	struct enable_if<true, T> { typedef T type; };


	#pragma inline=forced
	template<typename T>
	typename stdx::enable_if<!stdx::is_signed<T>::value, bool>::type is_negative(T)
	{
		return false;
	}
	#pragma inline=forced
	template<typename T>
	typename stdx::enable_if<stdx::is_signed<T>::value, bool>::type is_negative(T value)
	{
		return value < 0;
	}

//Pinlist helpers

//NumberToMask<3>::value == 0b0111;
	template<uint32_t Num>
	struct NumberToMask
	{
		enum { value = 1 << (Num - 1) | NumberToMask<Num - 1>::value};
	};
	template<>
	struct NumberToMask<0>
	{
		enum { value = 0 };
	};

//MaskToPosition<0b0100>::value == 0x02;
	template<uint32_t mask>
	struct MaskToPosition
	{
		enum{ value = MaskToPosition<(mask >> 1)>::value + 1 };
	};
	template<>
	struct MaskToPosition<0x01>
	{
		enum{ value = 0 };
	};
//Need for Nullpin
	template<>
	struct MaskToPosition<0x00>
	{
		enum{ value = 0 };
	};
}

