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
#ifndef ITC_H
#define ITC_H

#include "stm8s.h"

namespace Mcudrv
{
	namespace System
	{
		enum ResetReason
		{
			Software_WWatchdog = 1U,
			IWatchdog = 1U << 1,
			IllOpcode = 1U << 2,
			Swim = 1U << 3,
			Emc = 1U << 4
		};

		FORCEINLINE
		static inline void Reset()
		{
			WWDG->CR = WWDG_CR_WDGA;
		}
		FORCEINLINE
		static inline ResetReason GetResetReason()
		{
			return (ResetReason)RST->SR;
		}
	}
	namespace Itc
	{
		enum Priority
		{
			prioLevel_1_low = 0x01,
			prioLevel_2_middle = 0x00,
			prioLevel_3_high = 0x03
		};
		FORCEINLINE
		void inline SetPriority(uint8_t vector, const Priority priority)
		{
			vector -= 2;
			volatile uint8_t* reg = &((volatile uint8_t*)ITC)[vector / 4];
			const uint8_t position = (vector % 4) * 2;
			*reg = (uint8_t)((*reg & ~(0x03 << position)) | priority << position);
		}

	}//Itc
}//Mcudrv

#endif // ITC_H

