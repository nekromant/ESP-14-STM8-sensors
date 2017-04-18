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
#include "gpio.h"

namespace Mcudrv {
	namespace Adcs {

	enum Mode {
		Mode8Bit,
		Mode10bit
	};

	enum Cfg {
		//		---=== ADC CR1 ===---
		ADCEnable = ADC1_CR1_ADON,			//Only this bit must be changed to start conversion
		ContMode = ADC1_CR1_CONT,

		//		---=== ADC CR2 ===---
		ScanMode = ADC1_CR2_SCAN << 8UL,

		//		LeftAlign = 0,
		//		RightAlign = ADC1_CR2_ALIGN << 8UL,

		//		ExTrigEnable
		//		Tim1TrgoEvent
		//		EtrPinEvent

		//		---=== ADC CR3 ===---
		BufferEnable = static_cast<uint32_t>(ADC1_CR3_DBUF) << 16UL
	};

	enum Div {		//Change when ADC Power down
		Div2,
		Div3,
		Div4,
		Div6,
		Div8,
		Div10,
		Div12,
		Div18
	};

	enum Channel {
		Ch0,
		Ch1,
		Ch2,
		Ch3,
		Ch4,
		Ch5,
		Ch6,
		Ch7,
		Ch8,
		Ch9,
		Ch10,
		Ch11,
		Ch12,
		Ch13,
		Ch14,
		Ch15
	};

	template<typename Pin>
	struct PinToCh;
#if defined (STM8S103) || defined (STM8S003)
	template<>
	struct PinToCh<Pc4> {
		enum {
			value = Ch2
		};
	};
	template<>
	struct PinToCh<Pd2> {
		enum {
			value = Ch3
		};
	};
	template<>
	struct PinToCh<Pd3> {
		enum {
			value = Ch4
		};
	};
	template<>
	struct PinToCh<Pd5> {
		enum {
			value = Ch5
		};
	};
	template<>
	struct PinToCh<Pd6> {
		enum {
			value = Ch6
		};
	};

#elif defined (STM8S105)
	//TODO: implement
#endif

	enum Ints {
		EndOfConv = ADC1_CSR_EOCIE,
		AnalogWatchdog = ADC1_CSR_AWDIE
	};

	template<Mode mode>
		struct AdcTraits
		{
			typedef uint16_t result_t;
			enum{max_value_ = 0x3FF};
		};

		template<>
		struct AdcTraits<Mode8Bit>
		{
			typedef uint8_t result_t;
			enum{max_value_ = 0xFF};
		};

		template<Mode mode = Mode10bit>
		class Adc
		{
		private:
		public:
			static const uint16_t* const buffer;

			enum{max_value = AdcTraits<mode>::max_value_};

//			static const uint16_t max_value;

			template<Cfg cfg, Div div>
			static void Init()
			{
				ADC1->CR1 = static_cast<uint8_t>(cfg) | (div << 4UL);
				ADC1->CR2 = cfg >> 8UL | (mode == Mode10bit) ? ADC1_CR2_ALIGN : 0UL;
				ADC1->CR3 = cfg >> 16UL;
			}

			static void EnableInterrupt(Ints mask)
			{
				ADC1->CSR |= mask;
			}
			static void DisableInterrupt(Ints mask)
			{
				ADC1->CSR &= ~mask;
			}

			template<uint16_t chmask, uint16_t low = 0, uint16_t high = 0x3FF>
			static void WatchdogInit()
			{
				*reinterpret_cast<volatile uint16_t*>(&ADC1->AWCRH) = chmask;
				if (mode == Mode10bit)
				{
					if (high != 0x3FF) *reinterpret_cast<volatile uint16_t*>(&ADC1->HTRH) = high;
					if (low) *reinterpret_cast<volatile uint16_t*>(&ADC1->LTRH) = low;
				}
				else // 8 bit mode
				{
					if (high != 0x3FF)
					{
						ADC1->HTRH = static_cast<uint8_t>(high);
//						ADC1->HTRL = 0x03;
					}
					if (low) 
					{
						ADC1->LTRH = static_cast<uint8_t>(low);
//						ADC1->LTRL = 0;
					}
				}
			}
			static void Enable()
			{
				ADC1->CR1 |= ADCEnable;
			}
			static void StartConversion()
			{
				ADC1->CR1 |= ADCEnable;
			}
			static void ClearConfig(Cfg cfg)
			{
				ADC1->CR1 &= ~cfg;
			}
			static void SetConfig(Cfg cfg)
			{
				ADC1->CR1 |= cfg;
			}
			static void Disable()
			{
				ADC1->CR1 &= ~ADCEnable;
			}
			static bool IsOverrun()
			{
				return ADC1->CR3 & ADC1_CR3_OVR;
			}
			
			template<Ints event>
			static bool IsEvent()
			{
				return ADC1->CSR & (event << 2UL);
			}
			FORCEINLINE
			static bool IsEvent(Ints event)
			{
				return ADC1->CSR & (event << 2UL);
			}

			template<Ints event>
			static void ClearEvent()
			{
				ADC1->CSR &= ~(event << 2UL);
			}
			FORCEINLINE
			static void ClearEvent(Ints event)
			{
				ADC1->CSR &= ~(event << 2UL);
			}

			FORCEINLINE
			static void SelectChannel(Channel ch)
			{
				ADC1->CSR &= ~ADC1_CSR_CH;
				ADC1->CSR |= ch;
			}

			FORCEINLINE
			static Channel GetSelectedChannel()
			{
				return Channel(ADC1->CSR & ADC1_CSR_CH);
			}

			#pragma inline=forced
			static typename AdcTraits<mode>::result_t ReadSample()
			{
				if(mode == Mode10bit) {
					return *reinterpret_cast<volatile uint16_t*>(&ADC1->DRH);
				}
				else {
					return ADC1->DRH;
				}
			}
			
			template<uint16_t mask>
			static void DisableSchmittTrigger()
			{
				ADC1->TDRL = static_cast<uint8_t>(mask);
				if(mask & 0xFF00) ADC1->TDRH = mask >> 8UL;
			}
			template<uint16_t mask>
			static void EnableSchmittTrigger()
			{
				ADC1->TDRL &= ~static_cast<uint8_t>(mask);
				if(mask & 0xFF00) ADC1->TDRH &= ~mask >> 8UL;
			}
			FORCEINLINE
			static void DisableSchmittTrigger(uint16_t mask)
			{
				ADC1->TDRL = static_cast<uint8_t>(mask);
				if(mask & 0xFF00) ADC1->TDRH = mask >> 8UL;
			}
			FORCEINLINE
			static void EnableSchmittTrigger(uint16_t mask)
			{
				ADC1->TDRL &= ~static_cast<uint8_t>(mask);
				if(mask & 0xFF00) ADC1->TDRH &= ~mask >> 8UL;
			}

		};

		template<>
		const uint16_t* const Adc<Mode10bit>::buffer = reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(&ADC1->DB0RH));		
		
		template<>
		const uint16_t* const Adc<Mode8Bit>::buffer = reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(&ADC1->DB0RH) - 1);

		typedef Adc<Mode10bit> Adc1;
	
	}
}
