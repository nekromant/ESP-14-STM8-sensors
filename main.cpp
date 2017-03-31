#include <intrinsics.h>

#define F_CPU 16000000UL

#include "stm8s.h"
#include "pinlist.h"
#include "delay.h"
#include "type_traits.h"
#include "spi.h"
#include "timers.h"

using namespace Mcudrv;

//Latency - number of first dropped cycles in autorepeat
//Scan function should be called with a period of 1..8 ms
template<typename Pinlist, typename Executor, uint8_t AutorepeatSpeed_ = 5, uint8_t Latency = 20>
class ButtonsPoll
{
private:
	static_assert(AutorepeatSpeed_ > 0 && AutorepeatSpeed_ < 7, "AutorepeatSpeed should be in a range of 1..6");
	enum {
		AutorepeatSpeed	= (1 << (8 - AutorepeatSpeed_))
	};
	static uint8_t autorepeat_timer, debounce_counter;
public:
	static void Init()
	{
		Pinlist::template SetConfig<GpioBase::In_Pullup>();
	}

	static void Scan()
	{
		uint8_t temp_buttons_state = ~Pinlist::Read() & stdx::NumberToMask<Pinlist::size>::value;
		if(temp_buttons_state != 0x00) {
			if(debounce_counter < (254 - Latency)) {
				++debounce_counter;
			}
			if(debounce_counter == 5) {
				debounce_counter = (254 - Latency);
				Executor::Call(temp_buttons_state);
			}
			if(++autorepeat_timer == AutorepeatSpeed) {
				autorepeat_timer = 0;
				if((debounce_counter > (253 - Latency)) && (debounce_counter != 0xFF)) {
					debounce_counter++;
				}
				if (debounce_counter == 0xFF) {
					Executor::Call(temp_buttons_state);
				}
			}
		}
		else {
			debounce_counter = 0;
		}
	}
};
template<typename Pinlist, typename Executor, uint8_t AutorepeatSpeed, uint8_t Latency>
uint8_t ButtonsPoll<Pinlist, Executor, AutorepeatSpeed, Latency>::autorepeat_timer;
template<typename Pinlist, typename Executor, uint8_t AutorepeatSpeed, uint8_t Latency>
uint8_t ButtonsPoll<Pinlist, Executor, AutorepeatSpeed, Latency>::debounce_counter;

struct LedMatrix
{
	typedef Pinlist<Pc3, Pb4, Pb5, Pc4> directPins;
	typedef Pc7 Latch;

	static void Init()
	{
		directPins::Write(0x0F);
		directPins::SetConfig<GpioBase::Out_OpenDrain>();
		Latch::SetConfig<GpioBase::Out_PushPull_fast>();
		Spi::Init(Spi::CfgDefault, Spi::Div2);
		for(int8_t i = 20; i >= 0; --i)
		{
			Write(i);
			delay_ms(50);
		}
	}

	static void Write(uint8_t val)
	{
		if(!val) {
			DriverWrite(0);
			directPins::Write(0x0F);
		}
		else if(val < 5) {
			DriverWrite(0);
			directPins::Write(~(1 << (val - 1)));
		}
		else {
			DriverWrite(1 << (val - 5));
			directPins::Write(0x0F);
		}
	}

private:
	static void DriverWrite(uint16_t val)
	{
		Spi::Write(val >> 8);
		Spi::Write(val);
		Spi::WaitForFinish();
		Latch::Set();
		Latch::Clear();
	}
};

typedef Pinlist<Pa1, SequenceOf<2> > buttons;

struct Execute
{
private:
	enum {
		MinCompVal = 8,
		DutyCycle = 64
	};
public:
	static void Call(uint8_t state)
	{
		enum {
			ButtonMinus = 1U << 0,
			ButtonPlus = 1U << 1
		};
		static uint8_t val;
		switch(state) {
		case ButtonMinus:
			if(val > 0) {
				LedMatrix::Write(--val);
				WritePulseWidth(val);
			}
			break;
		case ButtonPlus:
			if(val < 20) {
				LedMatrix::Write(++val);
				WritePulseWidth(val);
			}
			break;
		}
	}
private:
	static void WritePulseWidth(uint8_t val)
	{
		using namespace T2;
		//0 : Off
		if(!val) {
			Timer2::WriteCompareByte<Timers::Ch1>(0);
		}
		//duty cycle 1/16
		//1 :	0.5 us		9 : 128 us
		//2 : 1 us			10 : 256 us
		//3 : 2 us			11 : 512 us
		//4 : 4 us			12 : 1 ms
		//5 : 8 us			13 : 2 ms
		//6 : 16 us			14 : 4 ms
		//7 : 32 us			15 : 8 ms
		//8 : 64 us			16 : 16 ms
		else if(val < 17) {
			Timer2::WriteCompareByte<Timers::Ch1>(MinCompVal);
			Timer2::WriteAutoReload(MinCompVal * DutyCycle);
			Timer2::WritePrescaler(Div(val - 1));
		}
		//duty cycle 1/8
		//16 : 32 ms
		//17 : 64 ms
		//18 : 128 ms
		//19 : 256 ms
		else if(val < 21) {
			val = MinCompVal << (val - 16);
			Timer2::WriteCompareByte<Timers::Ch1>(val);
			Timer2::WriteAutoReload(val * (DutyCycle/2));
		}
	}
};

typedef ButtonsPoll<buttons, Execute, 3, 5> Keyboard;

typedef Pd4 MosfetDriver;

int main()
{
	SysClock::SetHsiDivider(SysClock::Div1); //set F_CPU to 16 MHz
	GpioA::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioB::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioC::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioD::WriteConfig<0xFF, GpioBase::In_Pullup>();
	LedMatrix::Init();
	Keyboard::Init();
	MosfetDriver::SetConfig<GpioBase::Out_PushPull_fast>();
	using namespace T2;
	Timer2::Init(Div_1, Cfg(ARPE | CEN));
	Timer2::SetChannelCfg<Timers::Ch1, Output, Out_PWM_Mode1>();
	Timer2::WriteCompareByte<Timers::Ch1>(0);
	Timer2::ChannelEnable<Timers::Ch1>();
	Timer2::Enable();
	while(true) {
		delay_ms(4);
		Keyboard::Scan();
	}
}


