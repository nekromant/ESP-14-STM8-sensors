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
#include "stm8s.h"

#ifndef F_CPU
#error "F_CPU should be defined"
#endif

namespace Mcudrv {
namespace SysClock {
	enum RefSource
	{
		HSE = 0xB4,
		HSI = 0xE1, //Default
		LSI = 0xD2
	};
	enum HsiDiv
	{
		Div1, Div2, Div4, Div8 //Div8 - Default
	};

	#pragma inline=forced
	inline static void Select(const RefSource ref)
	{
		CLK->SWCR |= CLK_SWCR_SWEN;
		CLK->SWR = ref;
		while(!(CLK->SWCR & CLK_SWCR_SWBSY))
			;
		while(CLK->SWCR & CLK_SWCR_SWBSY)
			;
		CLK->SWCR = 0;
	}
	#pragma inline=forced
	inline static void SetHsiDivider(const HsiDiv div)
	{
		CLK->CKDIVR = div << 3U;
	}

}//Clock
}//Mcudrv
