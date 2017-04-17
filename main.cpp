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

#define REG_INPUT_START 1
#define REG_INPUT_NREGS 5
#define DEVICE_ADDRESS 42

#define SENSOR_POLL_PERIOD 30 //in secs

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];

typedef Twis::SoftTwi<Twis::Standard, Pb4, Pb5> i2c;
typedef Twis::Bh1750<i2c> LSensor;
typedef Twis::Bmp280<i2c> PSensor;
typedef Twis::Hdc1080<i2c> HSensor;

volatile static uint16_t readSensorsTimer;

int main()
{
//  SysClock::SetHsiDivider(SysClock::Div1); //set F_CPU to 16 MHz
  GpioA::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioB::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioC::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioD::WriteConfig<0xFF, GpioBase::In_Pullup>();

  T4::Timer4::Init(T4::Div_128, T4::Cfg(T4::ARPE | T4::CEN)); // 61Hz
  T4::Timer4::EnableInterrupt();

  i2c::Init();
  LSensor::Init();
  PSensor::Init();

  PSensor::PT pt;
  HSensor::HT ht;

  eMBInit(MB_RTU, DEVICE_ADDRESS, 0, 9600, MB_PAR_NONE);
  enableInterrupts();
  eMBEnable();
  while(true) {
      eMBPoll( );
      if(readSensorsTimer == SENSOR_POLL_PERIOD * 61) { // Secs * (timer frequency)
        readSensorsTimer = 0;
        PSensor::GetValues(pt);
        HSensor::GetValues(ht);
        usRegInputBuf[0] = LSensor::Read();
        usRegInputBuf[1] = uint16_t(pt.pressure - 60000UL);
        usRegInputBuf[2] = pt.temperature;
        usRegInputBuf[3] = ht.humidity;
        usRegInputBuf[4] = ht.temperature;
      }
  }
}

extern "C"
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
  eMBErrorCode eStatus = MB_ENOERR;
  int iRegIndex;
  uint16_t* regBuffer16 = (uint16_t*)pucRegBuffer;
  if((usAddress >= REG_INPUT_START) && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS)) {
    iRegIndex = (int)(usAddress - usRegInputStart);
    while(usNRegs > 0) {
      *regBuffer16++ = usRegInputBuf[iRegIndex];
      iRegIndex++;
      usNRegs--;
    }
  }
  else {
    eStatus = MB_ENOREG;
  }

  return eStatus;
}

INTERRUPT_HANDLER(Timer4_ISR, TIM4_OVR_UIF_vector - 2)
{
  T4::Timer4::ClearIntFlag();
  ++readSensorsTimer;
}
