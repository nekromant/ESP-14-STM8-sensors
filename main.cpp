#include <intrinsics.h>

#define F_CPU 16000000UL

#include "stm8s.h"
#include "pinlist.h"
#include "delay.h"
#include "type_traits.h"
#include "i2c.h"
#include "uart.h"
#include "timers.h"

using namespace Mcudrv;

typedef Uarts::UartIrq<128> Uart;

template void Uart::TxISR();
template void Uart::RxISR();

typedef Twis::SoftTwi<Twis::Standard, Pb4, Pb5> i2c;
typedef Twis::Bh1750<i2c> LightSensor;
typedef Twis::Bmp180<i2c> PressureSensor;

int main()
{
	SysClock::SetHsiDivider(SysClock::Div1); //set F_CPU to 16 MHz
	GpioA::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioB::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioC::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioD::WriteConfig<0xFF, GpioBase::In_Pullup>();
	{
		using namespace Uarts;
		::Uart::Init<DefaultCfg, 115200>();
	}
	i2c::Init();
	LightSensor::Init();
	PressureSensor::Init();
	enableInterrupts();
//	PressureSensor::PrintCalArray<Uart>();
	PressureSensor::PT pt;
	while(true) {
		delay_ms(1000);
		PressureSensor::GetValues(pt);
		Uart::Puts("\r                    \r");
		Uart::Puts(LightSensor::Read());
		Uart::Puts("  ");
		Uart::Puts(pt.pressure);
		Uart::Puts(" Pa ");
		Uart::Puts(pt.temperature);
		Uart::Puts(" C");
	}
}


