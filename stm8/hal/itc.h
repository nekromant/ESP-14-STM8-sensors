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

