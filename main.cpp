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

typedef Uarts::UartIrq<512,16> Uart;

template void Uart::TxISR();
template void Uart::RxISR();

typedef Twis::SoftTwi<Twis::Standard, Pb4, Pb5> i2c;
typedef Twis::Bh1750<i2c> LSensor;
typedef Twis::Bmp280<i2c> PSensor;
typedef Twis::Hdc1080<i2c> HSensor;

int main()
{
	SysClock::SetHsiDivider(SysClock::Div1); //set F_CPU to 16 MHz
	GpioA::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioB::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioC::WriteConfig<0xFF, GpioBase::In_Pullup>();
	GpioD::WriteConfig<0xFF, GpioBase::In_Pullup>();
  Uart::Init<Uarts::DefaultCfg, 115200>();
	i2c::Init();
  LSensor::Init();
  PSensor::Init();
	enableInterrupts();
  Uart::Newline();
  PSensor::PT pt;
  HSensor::HT ht;
  uint8_t buf[8];
  while(true) {
    delay_ms(1000);
    PSensor::GetValues(pt);
    HSensor::GetValues(ht);
    uint16_t lum = LSensor::Read();

    Uart::Puts("<span style='color:#F4FA58'>");
    Uart::Puts("<div style='font-size: 0.2em'>BH1750</div>");
    Uart::Puts(lum);
    Uart::Puts("<span style='font-size: 0.5em'>lx</span>");
    Uart::Puts("</span>");

    Uart::Puts("<span style='color:#F79F81'>");
    Uart::Puts("<div style='font-size: 0.2em'>BMP280</div>");
    Uart::Puts(pt.pressure);
    Uart::Puts("<span style='font-size: 0.5em'>Pa</span>  ");
    Uart::Puts((const char*)io::InsertDot(pt.temperature, 2, buf));
    Uart::Puts("<span style='font-size: 0.5em'>°C</span>");
    Uart::Puts("</span>");

    Uart::Puts("<span style='color:#F5ECCE'>");
    Uart::Puts("<div style='font-size: 0.2em'>HDC1080</div>");
    Uart::Puts((const char*)io::InsertDot(ht.humidity, 1, buf));
    Uart::Puts("<span style='font-size: 0.5em'>%</span>  ");
    Uart::Puts((const char*)io::InsertDot(ht.temperature, 1, buf));
    Uart::Puts("<span style='font-size: 0.5em'>°C</span>");
    Uart::Puts("</span>");

    Uart::Newline();

	}
}


