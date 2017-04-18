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
#include "type_traits.h"

namespace Mcudrv
{

	namespace Exti
	{
		enum Port
		{
			Porta,
			Portb,
			Portc,
			Portd,
			Porte
		};

		enum Cfg
		{
			LowLevel,
			RisingEdge,
			FallingEdge,
			RisingFallingEdge
		};
		
		#pragma inline=forced
		template<Port port, Cfg cfg>
		void SetExtIntMode()
		{
			if (port == Porte) EXTI->CR2 |= cfg;
			else EXTI->CR1 |= cfg << (port * 2);
		}
		#pragma inline=forced
		template<Port port>
		void ClearExtIntMode()
		{
			if (port == Porte) EXTI->CR2 &= ~(0x0F);
			else EXTI->CR1 &= ~(0x0F << (port * 2));
		}
		
		#pragma inline=forced
		template<typename port, Cfg cfg>
		void SetExtIntMode()
		{
			SetExtIntMode<static_cast<Exti::Port>(port::id), cfg>();
		}
	}

	struct GpioBase
	{
		enum Cfg
		{
			In_float,
			In_float_int,
			In_Pullup,
			In_Pullup_int,
			Out_OpenDrain,
			Out_OpenDrain_fast,
			Out_PushPull,
			Out_PushPull_fast
		};
	};

	template <uint16_t baseaddr, uint8_t ID>
	class Gpio : GpioBase
	{
	private:
		#pragma inline=forced
		static GPIO_TypeDef* GetBase()
		{
			return reinterpret_cast<GPIO_TypeDef*>(baseaddr);
		}
	public:
		typedef Gpio Base;
		typedef uint8_t DataT;
		enum{ Width = 8 };
		enum{ id = ID };

		#pragma inline=forced
		template <uint8_t mask, Cfg cfg>
		static void SetConfig()
		{
			if (cfg & 0x01)
			{
				GetBase()->CR2 |= mask;
			}
			else GetBase()->CR2 &= ~mask;
			if ((cfg >> 1) & 0x01)
			{
				GetBase()->CR1 |= mask;
			}
			else GetBase()->CR1 &= ~mask;
			if ((cfg >> 2) & 0x01)
			{
				GetBase()->DDR |= mask;
			}
			else GetBase()->DDR &= ~mask;
		}

		#pragma inline=forced
		template <uint8_t mask, Cfg cfg>
		static void WriteConfig()
		{
			GetBase()->CR2 = ((cfg & 0x01) * 0xff) & mask;
			GetBase()->CR1 = (((cfg >> 1) & 0x01) * 0xff) & mask;
			GetBase()->DDR = (((cfg >> 2) & 0x01) * 0xff) & mask;
		}
		
		#pragma inline=forced
		template <uint8_t value>
		static void Write()
		{
			GetBase()->ODR = value;
		}
		#pragma inline=forced
		static void Write(uint8_t value)
		{
			GetBase()->ODR = value;
		}

		#pragma inline=forced
		template <uint8_t mask>
		static void Set()
		{
			GetBase()->ODR |= mask;
		}
		#pragma inline=forced
		static void Set(uint8_t mask)
		{
			GetBase()->ODR |= mask;
		}

		#pragma inline=forced
		template <uint8_t mask>
		static void Clear()
		{
			GetBase()->ODR &= ~mask;
		}
		#pragma inline=forced
		static void Clear(uint8_t mask)
		{
			GetBase()->ODR &= ~mask;
		}

		#pragma inline=forced
		template <uint8_t mask>
		static void Toggle()
		{
			GetBase()->ODR ^= mask;
		}
		#pragma inline=forced
		static void Toggle(uint8_t mask)
		{
			GetBase()->ODR ^= mask;
		}
		
		#pragma inline=forced
		static uint8_t Read()
		{
			return GetBase()->IDR;
		}
		#pragma inline=forced
		static uint8_t ReadODR()
		{
			return GetBase()->ODR;
		}
		#pragma inline=forced
		template<uint8_t clearmask, uint8_t setmask>
		static void ClearAndSet()
		{
			const DataT value = setmask & ~clearmask;
			GetBase()->ODR = value;
		}
		#pragma inline=forced
		static void ClearAndSet(uint8_t clearmask, uint8_t setmask)
		{
			const DataT value = setmask & ~clearmask;
			GetBase()->ODR = value;
		}
	};

	struct GpioNull : GpioBase
	{
		typedef GpioNull Base;
		typedef uint8_t DataT;
		enum{ Width = 8 };
		enum{ id = 0xFF };
		#pragma inline=forced
		template <uint8_t mask, Cfg cfg>
		static void SetConfig()
		{	}
		#pragma inline=forced
		template <uint8_t mask, Cfg cfg>
		static void WriteConfig()
		{	}
		#pragma inline=forced
		template <uint8_t value>
		static void Write()
		{	}
		#pragma inline=forced
		static void Write(uint8_t /*value*/)
		{	}
		#pragma inline=forced
		template <uint8_t mask>
		static void Set()
		{	}
		#pragma inline=forced
		static void Set(uint8_t /*mask*/)
		{	}
		#pragma inline=forced
		template <uint8_t mask>
		static void Clear()
		{	}
		#pragma inline=forced
		static void Clear(uint8_t /*mask*/)
		{	}
		#pragma inline=forced
		template <uint8_t mask>
		static void Toggle()
		{	}
		#pragma inline=forced
		static void Toggle(uint8_t /*mask*/)
		{	}
		#pragma inline=forced
		template<uint8_t clearmask, uint8_t setmask>
		static void ClearAndSet()
		{	}
		#pragma inline=forced
		static uint8_t Read()
		{
			return 0;
		}
		#pragma inline=forced
		static uint8_t ReadODR()
		{
			return 0;
		}
	};

#define PORTDEF(x,y) typedef Gpio<GPIO##x##_BaseAddress, y> Gpio##x	

	PORTDEF(A, 0);
	PORTDEF(B, 1);
	PORTDEF(C, 2);
	PORTDEF(D, 3);
	PORTDEF(E, 4);
	PORTDEF(F, 5);
	PORTDEF(G, 6);
	PORTDEF(H, 7);
	PORTDEF(I, 8);

	template <typename Port_, uint8_t Mask_>
	class TPin
	{
	public:
		typedef Port_ Port;
		enum
		{
			mask = Mask_,
			position = stdx::MaskToPosition<mask>::value,
			port_id = Port::id
		};
		static const bool Exist = mask;
		#pragma inline=forced
		template <GpioBase::Cfg cfg>
		static void SetConfig()
		{
			Port::template SetConfig<mask, cfg>();
		}
		
		#pragma inline=forced
		static void Set()
		{
			Port::template Set<mask>();
		}
		#pragma inline=forced
		static void SetOrClear(bool cond)
		{
			if (cond) Port::template Set<mask>();
			else Port::template Clear<mask>();
		}
		#pragma inline=forced
		static void Clear()
		{
			Port::template Clear<mask>();
		}
		#pragma inline=forced
		static void Toggle()
		{
			Port::template Toggle<mask>();
		}
		#pragma inline=forced
		static bool IsSet()
		{
			return Port::Read() & mask;
		}
		#pragma inline=forced
		static bool IsODRSet()
		{
			return Port::ReadODR() & mask;
		}
	};

	template<typename Pin>
	struct InvertedPin : public Pin
	{
#pragma inline=forced
		static void Set()
		{
			Pin::Clear();
		}
#pragma inline=forced
		static void Clear()
		{
			Pin::Set();
		}
	};

	static inline void ReleaseSwimPin_Pd1()
	{
		CFG->GCR |= CFG_GCR_SWD;
	}

#define PINSDEF(x,y)	typedef TPin<Gpio##x, 0x01> P##y##0;\
						typedef TPin<Gpio##x, 0x02> P##y##1;\
						typedef TPin<Gpio##x, 0x04> P##y##2;\
						typedef TPin<Gpio##x, 0x08> P##y##3;\
						typedef TPin<Gpio##x, 0x10> P##y##4;\
						typedef TPin<Gpio##x, 0x20> P##y##5;\
						typedef TPin<Gpio##x, 0x40> P##y##6;\
						typedef TPin<Gpio##x, 0x80> P##y##7
	PINSDEF(A,a);
	PINSDEF(B,b);
	PINSDEF(C,c);
	PINSDEF(D,d);
	PINSDEF(E,e);
	PINSDEF(F,f);
	PINSDEF(G,g);
	PINSDEF(H,h);
	PINSDEF(I,i);
	typedef TPin<GpioNull, 0x0> Nullpin;

	enum {
		P0 = 0x01,
		P1 = 0x02,
		P2 = 0x04,
		P3 = 0x08,
		P4 = 0x10,
		P5 = 0x20,
		P6 = 0x40,
		P7 = 0x80
	};
} //Mcudrv
