#include <intrinsics.h>

#include "stm8s.h"
#include "pinlist.h"
#include "delay.h"
#include "type_traits.h"
#include "i2c.h"
#include "uart.h"
#include "timers.h"

#include "mb.h"
#include "mbport.h"

using namespace Mcudrv;

//typedef Uarts::UartIrq<512,16> Uart;

//template void Uart::TxISR();
//template void Uart::RxISR();

#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];

typedef Twis::SoftTwi<Twis::Standard, Pb4, Pb5> i2c;
typedef Twis::Bh1750<i2c> LSensor;
typedef Twis::Bmp280<i2c> PSensor;
typedef Twis::Hdc1080<i2c> HSensor;


int main()
{
//  SysClock::SetHsiDivider(SysClock::Div1); //set F_CPU to 16 MHz
  GpioA::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioB::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioC::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioD::WriteConfig<0xFF, GpioBase::In_Pullup>();
//  Uart::Init<Uarts::DefaultCfg, 9600>();
  i2c::Init();
  LSensor::Init();
  PSensor::Init();
  eMBErrorCode    eStatus;

  eStatus = eMBInit(MB_RTU, 0x0A, 0, 0, MB_PAR_NONE);
  enableInterrupts();

  /* Enable the Modbus Protocol Stack. */
  eStatus = eMBEnable();

  for(;;) {
      eMBPoll( );
      /* Here we simply count the number of poll cycles. */
      usRegInputBuf[0]++;
  }
//  Uart::Newline();
/*  PSensor::PT pt;
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
    */
}

extern "C"
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
  eMBErrorCode eStatus = MB_ENOERR;
  int iRegIndex;
  if((usAddress >= REG_INPUT_START) && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS)) {
    iRegIndex = (int)(usAddress - usRegInputStart);
    while(usNRegs > 0) {
      *pucRegBuffer++ = (unsigned char)(usRegInputBuf[iRegIndex] >> 8);
      *pucRegBuffer++ = (unsigned char)(usRegInputBuf[iRegIndex] & 0xFF);
      iRegIndex++;
      usNRegs--;
    }
  }
  else {
    eStatus = MB_ENOREG;
  }

  return eStatus;
}


