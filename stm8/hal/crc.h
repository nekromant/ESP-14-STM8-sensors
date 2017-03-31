#pragma once
#ifndef CRC_H
#define CRC_H

#include "stdint.h"

namespace Mcudrv {
namespace Crc {

//Maxim-Dallas computation (X8 + X5 + X4 + 1)
class Crc8
{
private:
	typedef Crc8 Self;
	static const uint8_t table[256];
	uint8_t crc_;
public:
//	Crc8(uint8_t init = 0) : crc_(init)
//	{	}
    void Init(uint8_t init)
    {
      crc_ = init;
    }
	Self& Reset(uint8_t init = 0)
	{
		crc_ = init;
		return *this;
	}
	Self& operator()(uint8_t value)
	{
		crc_ = table[crc_ ^ value];
		return *this;
	}
	Self& operator()(const uint8_t* buf, uint8_t len)
	{
		for(uint8_t i = 0; i < len; ++i)
		{
			crc_ = table[crc_ ^ buf[i]];
		}
		return *this;
	}
	uint8_t GetResult()
	{
		return crc_;
	}
};

namespace NoLUT {

struct Crc8_Algo1
{
#pragma inline=forced
	static void Evaluate(uint8_t& crc, uint8_t inByte)
	{
		for(uint8_t i = 8; i; --i) {
			uint8_t mix = (crc ^ inByte) & 0x01;
			crc >>= 1;
			if(mix) {
				crc ^= 0x8C;
			}
			inByte >>= 1;
		}
	}
};
struct Crc8_Algo2
{
    #pragma inline=forced
	static void Evaluate(uint8_t& crc, uint8_t inByte)
	{
		for(char i = 0; i < 8; inByte = inByte >> 1, ++i)
			if((inByte ^ crc) & 1) {
				crc = ((crc ^ 0x18) >> 1) | 0x80;
			}
			else crc = (crc >> 1) & ~0x80;
	}
};

template<typename Algo = Crc8_Algo1>
class Crc8
{
private:
	typedef Crc8 Self;
	uint8_t crc_;
public:
//  Crc8(uint8_t init = 0) : crc_(init)
//	{	}
    void Init(uint8_t init)
    {
      crc_ = init;
    }
	Self& Reset(uint8_t init = 0)
	{
		crc_ = init;
		return *this;
	}
	Self& operator()(uint8_t value)
	{
		Algo::Evaluate(crc_, value);
		return *this;
	}
	Self& operator()(const uint8_t* buf, uint8_t len)
	{
		for(uint8_t i = 0; i < len; ++i)
		{
          operator()(buf[i]);
//        Algo::Evaluate(crc_, buf[i]);
		}
		return *this;
	}
	uint8_t GetResult()
	{
		return crc_;
	}
};

}//NoLUT

typedef NoLUT::Crc8<NoLUT::Crc8_Algo1> Crc8_NoLUT;

}//Crc
}//Mcudrv

#endif // CRC_H

