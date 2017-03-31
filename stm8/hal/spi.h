#ifndef SPI_H
#define SPI_H
#include "stm8s.h"
#include "gpio.h"

namespace Mcudrv {

struct SpiBase
{
	enum Cfg {
		//CR1
		CPHA = SPI_CR1_CPHA,
		CPOL = SPI_CR1_CPOL,
		MSTR = SPI_CR1_MSTR,
		SPE = SPI_CR1_SPE,
		LSBFIRST = SPI_CR1_LSBFIRST,
		//CR2
		BDM = SPI_CR2_BDM << 8,
		BDOE = SPI_CR2_BDOE << 8,
		CRCEN = SPI_CR2_CRCEN << 8,
		CRCNEXT = SPI_CR2_CRCNEXT << 8,
		RXONLY = SPI_CR2_RXONLY << 8,
		SSM = SPI_CR2_SSM << 8,
		SSI = SPI_CR2_SSI << 8,
		CfgDefault = MSTR | SSM | SSI
	};
	//Should not be changed when comm. ongoing
	enum { DIVOFFSET = 3 };
	enum Div {
		Div2,
		Div4,
		Div8,
		Div16,
		Div32,
		Div64,
		Div128,
		Div256
	};
	enum Ints {
		IntTXE = SPI_ICR_TXEI,
		IntRXNE = SPI_ICR_RXEI,
		IntERR = SPI_ICR_ERRIE,
		IntWKUP = SPI_ICR_WKIE
	};
	enum Events {
		EvBUSY = SPI_SR_BSY,
		EvOVR = SPI_SR_OVR,
		EvMODF = SPI_SR_MODF,
		EvCRCERR = SPI_SR_CRCERR,
		EvWKUP = SPI_SR_WKUP,
		EvTXE = SPI_SR_TXE,
		EvRXNE = SPI_SR_RXNE
	};
};
static inline SpiBase::Cfg operator|(SpiBase::Cfg lhs, SpiBase::Cfg rhs)
{
	return static_cast<SpiBase::Cfg>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
static inline SpiBase::Ints operator|(SpiBase::Ints lhs, SpiBase::Ints rhs)
{
	return static_cast<SpiBase::Ints>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
//static inline SpiBase::Events operator|(SpiBase::Events lhs, SpiBase::Events rhs)
//{
//	return static_cast<SpiBase::Events>(static_cast<int>(lhs) | static_cast<int>(rhs));
//}

class Spi : public SpiBase
{
public:
	static void Init(Cfg cfg, Div div)
	{
		GpioC::SetConfig<P5 | P6, GpioBase::Out_PushPull_fast>();
		Pc7::SetConfig<GpioBase::In_Pullup>();
		SPI->CR1 = (cfg & 0xFF) | (uint8_t)(div << DIVOFFSET);
		if(cfg & 0xFF00) {
			SPI->CR2 = cfg >> 8;
		}
		Enable();
	}
	static void Enable()
	{
		SPI->CR1 |= SPE;
	}
	static void Disable()
	{
		SPI->CR1 &= ~SPE;
	}
	static void EnableInterrupt(Ints ints)
	{
		SPI->ICR |= ints;
	}
	static void DisableInterrupt(Ints ints)
	{
		SPI->ICR &= ~ints;
	}
	static void ClearEvent(Events ev)
	{
		SPI->SR = ~ev;
	}
	static bool IsEvent(Events ev)
	{
		return SPI->SR & ev;
	}

	static uint8_t Read()
	{
		while(!IsEvent(EvRXNE))
			;
		return SPI->DR;
	}
	static void Write(uint8_t val)
	{
		while(!IsEvent(EvTXE))
			;
		SPI->DR = val;
	}
	static uint8_t ReadWrite(uint8_t val)
	{
		Write(val);
		return Read();
	}
	static void WaitForFinish()
	{
		__no_operation();
		__no_operation();
		while(IsEvent(EvBUSY))
			;
	}
};

template<typename DataPin, typename ClkPin, typename LatchPin, uint8_t SizeInBytes = 1>
class SerialReg
{
private:
	enum { SizeInBits = SizeInBytes * 8 };
	typedef typename stdx::SelectSize<SizeInBits>::type DataType;
public:
	static void Init()
	{
		DataPin::template SetConfig<GpioBase::Out_PushPull_fast>();
		ClkPin::template SetConfig<GpioBase::Out_PushPull_fast>();
		LatchPin::template SetConfig<GpioBase::Out_PushPull_fast>();
	}
	static void Send(DataType data, bool NoLatch = false)
	{
		for(uint8_t i = 0; i < SizeInBits; ++i) {
			DataPin::SetOrClear(data & (1UL << (SizeInBits - 1)));
			ClkPin::Set();
			data <<= 1;
			ClkPin::Clear();
		}
		if(!NoLatch) {
			Latch();
		}
	}
	static void Latch()
	{
		LatchPin::Set();
		__no_operation();
		LatchPin::Clear();
	}

};

template<typename Spi, typename Cs>
class Max6675
{
private:
	enum { TC_EXIST_MASK = 0x04 };
public:
	static void Init()
	{
		Cs::Set();
		Cs::template SetConfig<GpioBase::Out_PushPull_fast>();
	}
	static int16_t Read()
	{
		Cs::Clear();
		if(Spi::IsEvent(Spi::EvRXNE)) {
			(void)Spi::Read();
		}
		uint16_t value = Spi::ReadWrite(0xFF) << 8;
		value |= Spi::ReadWrite(0xFF);
		Spi::WaitForFinish();
		Cs::Set();
		if(!(value & TC_EXIST_MASK)) {
			value >>= 3;
			if(value > (2 * 4)) {
				value -= (3 * 4); //coarse compensation of IC selfheating
			}
			else value = 0;
			return value;
		}
		else {
			return -1;
		}
	}
};

}//Mcudrv

#endif // SPI_H
