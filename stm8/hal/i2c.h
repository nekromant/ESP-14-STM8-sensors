//Software implementation

#pragma once
#ifndef I2C_H
#define I2C_H

#include "stm8s.h"
#include "gpio.h"
#include "clock.h"
#include "timers.h"
#include "delay.h"

//#define FAST_I2C_MODE

namespace Mcudrv
{
namespace Twis
{
	enum
	{
		BaseAddrLM75 = 0x48,
		BaseAddr24C = 0x50,
		BaseAddrBH1750 = 0x23,
		BaseAddrBMP180 = 0x77
	};
	enum Mode
	{
		Standard,
		Fast
	};
	enum AddrType
	{
		Addr7bit,
		Addr10bit
	};
	enum StopMode
	{
		Stop,
		NoStop
	};
	enum AckState
	{
		NoAck, Ack
	};

	//Fast mode (400kHz) is only valid if Fcpu = 16 MHz
	template<Mode mode = Standard, typename Scl = Pe1, typename Sda = Pe2>
	class SoftTwi
	{
	private:
		static void Delay()
		{
			if(mode == Standard)
				delay_us<3>();
			else
			{
				__no_operation();
				__no_operation();
			}
		}
		#pragma inline=forced
		static bool Release()
		{
			for(uint8_t scl = 0; scl < 10; ++scl)
			{
				Scl::Clear();
				Delay();
				Scl::Set();
				Delay();
				if(Sda::IsSet())
				{
					Stop();
					return true;	//Sda released
				}
			}
			return false;	// Line is still busy
		}
	protected:
		#pragma inline=forced
		static void Start()
		{
			Sda::Clear();
			Delay();
			Scl::Clear();
			Delay();
		}
		#pragma inline=forced
		static void Stop()
		{
			Scl::Clear();
			Delay();
			Sda::Clear();
			Delay();
			Scl::Set();
			Delay();
			Sda::Set();
		}
		static AckState WriteByte(uint8_t data)
		{
			AckState ack = Ack;
			for(uint8_t i = 0; i < 8; ++i)
			{
				if((data & 0x80) == 0)
					Sda::Clear();
				else
					Sda::Set();
				Delay();
				Scl::Set();
				Delay();
				Scl::Clear();
				data <<= 1U;
			}
			Sda::Set();
			Delay();
			Scl::Set();
			Delay();
			if(Sda::IsSet())
				ack = NoAck;
			else
				ack = Ack;
			Scl::Clear();
			return ack;
		}
		static uint8_t ReadByte(AckState ackstate = Ack)
		{
			uint8_t data = 0;
			Sda::Set();
			for(uint8_t i = 0; i < 8; ++i)
			{
				data = (data << 1U);
				Scl::Set();
				Delay();
				if(Sda::IsSet()) data |= 0x01;
				Scl::Clear();
				Delay();
			}
			if(ackstate == Ack)
				Sda::Clear();
			else
				Sda::Set();
			Delay();
			Scl::Set();
			Delay();
			Scl::Clear();
			Delay();
			Sda::Set();
			return data;
		}

	public:
		static bool Init()
		{
			if((uint8_t)Scl::port_id == (uint8_t)Sda::port_id)
			{
				Scl::Port::Set((uint8_t)Scl::mask | (uint8_t)Sda::mask);
				Scl::Port::template SetConfig<(uint8_t)Scl::mask | (uint8_t)Sda::mask, GpioBase::Out_OpenDrain_fast>();
			}
			else
			{
				Scl::Set();
				Scl::template SetConfig<GpioBase::Out_OpenDrain_fast>();
				Sda::Set();
				Sda::template SetConfig<GpioBase::Out_OpenDrain_fast>();
			}
			if(!Sda::IsSet())
				return Release();			//Reset slave devices
			return true;	//Bus Ready
		}
		#pragma inline=forced
		static void Restart()
		{
			Sda::Set();
			Delay();
			Scl::Set();
			Delay();
		}
		static AckState Write(uint8_t addr, const uint8_t* buf, uint8_t length, bool noStop = false)
		{
			Start();
			AckState state = WriteByte(addr << 1);
			if(state == NoAck) goto End;
			while(length--)
			{
				if(WriteByte(*buf++) == NoAck)
					break;
			}
			if(!length) state = Ack;
		End:
			if(!noStop) Stop();
			return state;
		}
		static AckState Write(const uint8_t *buf, uint8_t length, bool noStop = false) //length of data (except address)
		{
			return Write(*buf, buf + 1, length, noStop);
		}
		static AckState Write(uint8_t addr, uint8_t data, bool noStop = false)
		{
			Start();
			AckState state = NoAck;
			if(WriteByte(addr << 1U) == Ack && WriteByte(data) == Ack)
				state = Ack;
			if(!noStop) Stop();
			return state;
		}
		static bool Read(uint8_t addr, uint8_t* buf, uint8_t length)
		{
			Start();
			bool result = false;
			if(WriteByte((addr << 1U) | 0x01))
			{
				while(--length)
				{
					*buf++ = ReadByte();
				}
				*buf = ReadByte(NoAck);
				result = true;
			}
			Stop();
			return result;
		}
	};

	template<typename Twi = SoftTwi<> >
	class Lm75
	{
	public:
		enum { BaseAddr = 0x48 };
		static int16_t Read(uint8_t devAddr = 0)
		{
			int16_t result;
			bool success = Twi::Read(BaseAddr | devAddr, (uint8_t*)&result, 2);
			return success ? result / 128 : 0;
		}
	};

	template<typename Twi = SoftTwi<> >
	class Eeprom24c
	{
	public:
		enum { BaseAddr = 0x50 };

	};

	template<typename Twi = SoftTwi<> >
	class Bh1750
	{
	public:
		enum { DevAddr = 0x23 };
		enum
		{
			BhPowerDown,
			BhPowerOn,
			BhReset = 0x07
		};
		enum Mode
		{
			ContHres = 0x10,// 1lx res. typ. 120ms (max 180ms)
			ContHres2,		// 0.5lx res. typ. 120ms
			ContLres		// 4lx res. typ. 16ms (max 24ms)
		};
		static AckState Init(Mode mode = ContHres)
		{
			return AckState(Twi::Write(DevAddr, BhPowerOn) && Twi::Write(DevAddr, mode));
		}
		static void SetMode(Mode mode)
		{
			return Twi::Write(DevAddr, mode);
		}

		static uint16_t Read()
		{
			uint8_t buf[2];
			if(!Twi::Read(DevAddr, buf, 2)) return 0;
			return uint16_t(buf[0] << 8U) | buf[1];
		}
	};

	template<typename Twi, uint8_t OversamplingFactor = 0>
	class Bmp180
	{
		static_assert(OversamplingFactor < 4, "Oversampling Factor must be in range 0..3");
	private:
		enum
		{
			BaseAddr = 0x77,
			Oss = OversamplingFactor,
			PMeasureDelay = Oss == 1 ? 8 :
							Oss == 2 ? 14 :
							Oss == 3 ? 26 : 5
		};
		enum RegMap
		{
			RegAC1 = 0xAA,
			RegTestID = 0xD0,
			RegControl = 0xF4,
			RegData = 0xF6,
			RegXlsb = 0xF8
		};
		enum CalValues
		{
			AC1, AC2, AC3, AC4, AC5, AC6,
			B1, B2,
			MB, MC, MD
		};
		enum ControlValue
		{
			CmdTemperature = 0x2E,
			CmdPressure = 0x34 | (Oss << 6),
		};

		static uint16_t calArr[11];

		static AckState SendCommand(ControlValue ctrl)
		{
			uint8_t data[2] = { RegControl, ctrl };
			return Twi::Write(BaseAddr, data, 2);
		}
		static uint16_t GetReg(RegMap reg)
		{
			using namespace Twis;
			uint16_t result;
			Twi::Write(BaseAddr, (uint8_t)reg, NoStop);
			Twi::Restart();
			Twi::Read(BaseAddr, (uint8_t*)&result, 2);
			return result;
		}
		static void GetCalValues()
		{
			for(uint8_t x = 0; x < 11; ++x)
			{
				calArr[x] = GetReg(RegMap(RegAC1 + x * 2));
			}
		}

	public:
		struct PT
		{
			uint32_t pressure;
			int16_t temperature;
		};
		static void Init()
		{
			GetCalValues();
		}
		template<typename Uart>
		static void PrintCalArray()
		{
			const uint8_t* const names[] = {
				"AC1", "AC2", "AC3", "AC4", "AC5", "AC6",
				"B1", "B2",
				"MB", "MC", "MD"
			};

			for(uint8_t i = 0; i < 11; ++i)
			{
				Uart::Puts(names[i]);
				Uart::Puts(": ");
				if(i < 3 || i > 5) Uart::Puts(int16_t(calArr[i]));
				else Uart::Puts(calArr[i]);
				Uart::Newline();
			}
		}

		static bool GetValues(PT& pt)
		{
			if(!SendCommand(CmdTemperature)) return false;
			delay_ms(5);
			uint16_t rawvalueT = GetReg(RegData);
			int32_t x1 = ((int32_t)rawvalueT - calArr[AC6]) * calArr[AC5] / (1U << 15);
			int32_t x2 = (int32_t)((int16_t)calArr[MC]) * (1U << 11) / (x1 + calArr[MD]);
			int32_t b5 = x1 + x2;
			pt.temperature = (b5 + 8) / (1U << 4);

			if(!SendCommand(CmdPressure)) return false;
			delay_ms(PMeasureDelay);
			int32_t rawvalueP = GetReg(RegData);
			if(Oss) rawvalueP = (rawvalueP << 8 | (GetReg(RegXlsb)) & 0xFF) >> (8 - Oss);
			int32_t b6 = b5 - 4000;
			x1 = ((int32_t)((int16_t)calArr[B2]) * (b6 * b6 / (1U << 12))) / (1U << 11);
			x2 = (int32_t)((int16_t)calArr[AC2]) * b6 / (1U << 11);
			int32_t x3 = x1 + x2;
			int32_t b3 = ((((int32_t)((int16_t)calArr[AC1]) * 4 + x3) << Oss) + 2) / 4;
			x1 = (int32_t)((int16_t)calArr[AC3]) * b6 / (1U << 13);
			x2 = ((int32_t)((int16_t)calArr[B1]) * ((b6 * b6) / (1U << 12))) / (1UL << 16);
			x3 = ((x1 + x2) + 2) / 4;
			uint32_t b4 = (int32_t)calArr[AC4] * (x3 + 32768U) >> 15U;
			uint32_t b7 = (rawvalueP - b3) * (50000U >> Oss);
			uint32_t p;
			if(b7 < 0x80000000UL)
			{
				p = (b7 * 2) / b4;
			}
			else
			{
				p = (b7 / b4) * 2;
			}
			x1 = (p >> 8) * (p >> 8);
			x1 = (x1 * 3038) >> 16;
			x2 = (-7357 * (int32_t)p) >> 16;
			p = p + ((x1 + x2 + 3791) >> 4);
			pt.pressure = p;
			return true;
		}
		static uint32_t GetPressure()
		{
			PT pt;
			GetValues(pt);
			return pt.pressure;
		}

		static int16_t GetTemperature()
		{
			SendCommand(CmdTemperature);
			delay_ms(5);
			uint16_t rawvalue = GetReg(RegData);
			int32_t x1 = ((int32_t)rawvalue - calArr[AC6]) * calArr[AC5] / (1U << 15);
			int32_t x2 = (int32_t)((int16_t)calArr[MC]) * (1U << 11) / (x1 + calArr[MD]);
			return (x1 + x2 + 8) / (1U << 4);
		}

	};
	template<typename Twi, uint8_t Oss>
	uint16_t Bmp180<Twi, Oss>::calArr[11];

}//i2c
}//Mcudrv

#endif // I2C_H

