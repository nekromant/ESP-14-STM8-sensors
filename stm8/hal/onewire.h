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
#ifndef ONEWIRE_H
#define ONEWIRE_H

#include "gpio.h"
#include "delay.h"
#include "crc.h"

namespace Mcudrv {

template<typename OwPin>
class OneWire
{
private:
	enum InstructionSet
	{
		CmdSearchRom = 0xF0,
		CmdReadRom = 0x33,
		CmdMatchRom = 0x55,
		CmdSkipRom = 0xCC
	};

	#pragma optimize=speed
	static void WriteBit(bool val)
	{
		disableInterrupts();
		OwPin::Clear();
		if(val) delay_us<10>();
		else delay_us<65>();
		OwPin::Set();
		enableInterrupts();
		if(val) delay_us<55>();
		else delay_us<5>();
	}
	#pragma optimize=speed
	static bool ReadBit()
	{
		disableInterrupts();
		OwPin::Clear();
		delay_us<5>();
		OwPin::Set();
		delay_us<10>();
		bool result = OwPin::IsSet();
		enableInterrupts();
		delay_us<55>();
		return result;
	}
	static uint8_t Search(uint8_t romArr[][8], const uint8_t romsMaxNumber, const bool checkRom)
	{
		uint8_t lastCollision = (!checkRom ? 0 : 64);
		uint8_t bitIndex, byteIndex, byteMask;
		uint8_t idBit, idBitComp;
		uint8_t  searchDirection = 0;
		Crc::Crc8 crc;
		uint8_t* prevRom = (uint8_t*)romArr;
		for(uint8_t romIndex = 0; romIndex < romsMaxNumber; ++romIndex)
		{
		// initialize for search
			uint8_t* const rom = &romArr[romIndex][0]; //8 byte buffer for current search
			uint8_t lastZero = 0;
			bitIndex = 1;	//Current bit search, start at 1, end at 64 (for convenience)
			byteIndex = 0; //Current byte search in
			byteMask = 1;
			crc.Reset();
			if(!Reset())
			{
				return 0; // No devices
			}
			Write(CmdSearchRom);
			do
			{
				// read a bit and its complement
				idBit = ReadBit();
				idBitComp = ReadBit();
				// check for no devices on 1-wire
				if(idBit && idBitComp)
				{
					return 0;
				}
				// All devices coupled have 0 or 1
				if(idBit != idBitComp)
					searchDirection = idBit;  // bit write value for search
				else	//collision here
				{
				// if this discrepancy is before the Last Discrepancy
				// on a previous next then pick the same as last time
					if(bitIndex < lastCollision)
						searchDirection = ((prevRom[byteIndex] & byteMask) > 0);
					else
						searchDirection = (bitIndex == lastCollision);
					if(!searchDirection)
					{
						lastZero = bitIndex;
					}
				}

				if(searchDirection)
					rom[byteIndex] |= byteMask;
				else
					rom[byteIndex] &= ~byteMask;

				WriteBit(searchDirection);

				++bitIndex;
				byteMask <<= 1;

				if(!byteMask)
				{
					crc(rom[byteIndex]);  // accumulate the CRC
					++byteIndex;
					byteMask = 1;
				}
			} while(byteIndex < 8);  // loop until through all ROM bytes 0-7
		// if the search was successful then
			if(bitIndex == 65)
			{
				if(crc.Get()) rom[0] |= 0x80;	//FamilyID MSB is correct crc flag
				if(!(lastCollision = lastZero))
				{
					return romIndex + 1;
				}
			}
			prevRom = rom;
		}
		return romsMaxNumber;
	}
public:
	union Descriptor
	{
		struct
		{
			uint8_t family;
			uint8_t id[6];
			uint8_t crc;
		};
		uint8_t data[8];
	};

	static void Init()
	{
		OwPin::Set();
		OwPin::template SetConfig<GpioBase::Out_OpenDrain_fast>();
	}
	//return true if presence exist
	#pragma optimize=speed
	static bool Reset()
	{
		OwPin::Clear();
		delay_us<480>();
		OwPin::Set();
		delay_us<70>();
		bool result = !OwPin::IsSet();
		delay_us<410>();
		return result;
	}
	static void Write(uint8_t val)
	{
		for(uint8_t i = 0; i < 8; ++i)
		{
			WriteBit(val & 0x01);
			val >>= 1;
		}
	}
	static void Write(const uint8_t* buf, uint16_t len)
	{
		for(uint16_t i = 0; i < len; ++i)
		{
			Write(buf[i]);
		}
	}
	static uint8_t Read()
	{
		uint8_t result = 0;
		for(uint8_t i = 0; i < 8; ++i)
		{
			result >>= 1;
			result |= ReadBit() << 7;
		}
		return result;
	}
	static uint8_t* Read(uint8_t* buf, uint16_t len)
	{
		for(uint16_t i = 0; i < len; ++i)
		{
			buf[i] = Read();
		}
		return buf;
	}
	static void SkipRom()
	{
		Write(CmdSkipRom);
	}
	static void Select(const Descriptor& desc)
	{
		MatchRom(desc.data);
	}
	static void MatchRom(const uint8_t* const rom)
	{
		Write(CmdMatchRom);
		Write(rom, 8);
	}
	static void MatchRom(const Descriptor& desc)
	{
		MatchRom(desc.data);
	}
	 /*Input - Empty array[x][8] will be filled with roms of devices found
	 Output - number of roms found*/
	static uint8_t Search(uint8_t romArr[][8], const uint8_t romsMaxNumber)
	{
		return Search(romArr, romsMaxNumber, false);
	}
	static uint8_t Search(Descriptor* desc, const uint8_t romsMaxNumber)
	{
		return Search(*(uint8_t(*)[][8])desc, romsMaxNumber, false);
	}

	/*Verify the device with the ROM number in rom buffer is present.
	 Return TRUE  : device verified present
			FALSE : device not present*/
	static bool Verify(const uint8_t* const romToCheck)
	{
		uint8_t rom[1][8];
		bool result = true;
		for(uint8_t i = 0; i < 8; ++i)
			rom[0][i] = romToCheck[i];
		if(Search(rom, 1, true))
		{
			for(uint8_t i = 0; i < 8; ++i)
				if(rom[0][i] != romToCheck[i])
					result = false;
		}
		return result;
	}
	static bool Verify(const Descriptor& romToCheck)
	{
		return Verify(romToCheck.data);
	}

	template<typename Ostream>
	static void PrintID(const uint8_t roms[][8], uint8_t devNumber)
	{
		for(uint8_t devn = 0; devn < devNumber; ++devn)
		{
			Ostream::Puts("Dev#");
			Ostream::Puts(devn);
			Ostream::Puts(" Family:");
			Ostream::Puts((uint8_t)roms[devn][0], 16);
			Ostream::Puts(" ID:");
			for(uint8_t i = 6; i; --i)
				Ostream::Puts((uint8_t)roms[devn][i], 16);
			Ostream::Puts(" CRC:");
			Ostream::Puts((uint8_t)roms[devn][7], 16);
			Ostream::Newline();
		}
	}
	template<typename Ostream>
	static void PrintID(const Descriptor* const desc, uint8_t devNumber)
	{
		PrintID<Ostream>(*(const uint8_t(*)[][8])desc, devNumber);
	}

};

//TODO: Specialisation for devMaxNumer = 1
template<typename Ow, uint8_t devMaxNumber>
class Ds18b20
{
private:
	typedef typename Ow::Descriptor Descriptor;
	static Descriptor romArr[devMaxNumber];
	static uint8_t devNumber;
	enum InstuctionSet
	{
		CmdConvert = 0x44,
		CmdRead = 0xBE
	};
public:
	static uint8_t Init()
	{
		devNumber = Ow::Search(romArr, devMaxNumber);
		return devNumber;
	}
	static bool Convert()
	{
		if(Ow::Reset())
		{
			Ow::SkipRom();
			Ow::Write(CmdConvert);
			return true;
		}
		else return false;
	}
	static uint16_t Get(uint8_t index)
	{
		if(Ow::Reset())
		{
			Ow::MatchRom(romArr[index]);
			Ow::Write(CmdRead);
			return Ow::Read() | Ow::Read() << 8;
		}
		else return 0xFFFF;
	}
	static uint16_t* Get(uint16_t* valArray)
	{
		for(uint8_t i = 0; i < devNumber; ++i)
		{
			valArray[i] = Get(i);
		}
		return valArray;
	}
	static Descriptor* GetId(uint8_t index)
	{
		return *romArr[index];
	}
	template<typename Ostream>
	static void PrintID()
	{
		Ow::template PrintID<Ostream>(romArr, devNumber);
	}
	static uint8_t GetSize()
	{
		return devNumber;
	}
};
template<typename Ow, uint8_t devMaxNumber>
typename Ds18b20<Ow, devMaxNumber>::Descriptor Ds18b20<Ow, devMaxNumber>::romArr[devMaxNumber];
template<typename Ow, uint8_t devMaxNumber>
uint8_t Ds18b20<Ow, devMaxNumber>::devNumber;

}//Mcudrv


#endif // ONEWIRE_H

