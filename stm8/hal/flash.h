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
