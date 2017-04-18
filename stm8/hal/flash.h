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

#include "stm8s.h"

namespace Mcudrv
{
	namespace Mem
	{
		enum MemType
		{
			Flash,
			Eeprom
		};
		
		FORCEINLINE	template<MemType>		//For Flash
		inline void Lock()
		{
			FLASH->IAPSR = ~FLASH_IAPSR_PUL;
		}
		FORCEINLINE	template<>
		inline void Lock<Eeprom>()
		{
			FLASH->IAPSR = ~FLASH_IAPSR_DUL;
		}
		FORCEINLINE inline void Lock(MemType type)
		{
			FLASH->IAPSR = ~(type == Flash ? FLASH_IAPSR_PUL : FLASH_IAPSR_DUL);
		}

		FORCEINLINE	template<MemType>		//For Flash
		inline void Unlock()
		{
			FLASH->PUKR = 0x56;
			FLASH->PUKR = 0xAE;
		}
		FORCEINLINE template<>
		inline void Unlock<Eeprom>()
		{
			FLASH->DUKR = 0xAE;
			FLASH->DUKR = 0x56;
		}
		FORCEINLINE inline void Unlock(MemType type)
		{
			if(type == Flash) {
				FLASH->PUKR = 0x56;
				FLASH->PUKR = 0xAE;
			}
			else {
				FLASH->DUKR = 0xAE;
				FLASH->DUKR = 0x56;
			}
		}

		FORCEINLINE template<MemType>
		inline bool IsUnlocked()		//For Flash
		{
			__no_operation();
			__no_operation();
			__no_operation();
			return FLASH->IAPSR & FLASH_IAPSR_PUL;
		}
		FORCEINLINE template<>
		inline bool IsUnlocked<Eeprom>()
		{
			__no_operation();
			__no_operation();
			__no_operation();
			return FLASH->IAPSR & FLASH_IAPSR_DUL;
		}
		FORCEINLINE inline bool IsUnlocked(MemType type)
		{
			__no_operation();
			__no_operation();
			__no_operation();
			return FLASH->IAPSR & (type == Flash ? FLASH_IAPSR_PUL : FLASH_IAPSR_DUL);
		}

		FORCEINLINE inline void SetWordProgramming()
		{
			FLASH->CR2 = FLASH_CR2_WPRG;
			FLASH->NCR2 = ~FLASH_NCR2_NWPRG;
		}

		FORCEINLINE inline void SetBlockProgramming()
		{
			FLASH->CR2 = FLASH_CR2_PRG;
			FLASH->NCR2 = ~FLASH_NCR2_NPRG;
		}

	}
}
