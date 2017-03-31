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
