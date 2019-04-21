//#include "uart.hpp"

#include <intrinsics.h>
#include "stm8s.h"
#include "pinlist.h"
#include "delay.h"
#include "type_traits.h"
#include "i2c.h"
#include "uart.h"
#include "timers.h"
#include "pinlist.h"


#include "mb.h"
#include "mbport.h"


using namespace Mcudrv;

#define REG_INPUT_START 1
#define REG_INPUT_NREGS 7
#define DEVICE_ADDRESS 42

#define SENSOR_POLL_PERIOD 30 //in secs

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];

typedef Twis::SoftTwi<Twis::Standard, Pb4, Pb5> i2c;
typedef Twis::Bh1750<i2c> LSensor;
typedef Twis::Bmp280<i2c> PSensor;
typedef Twis::Hdc1080<i2c> HSensor;


typedef Pinlist<Pc6, Pc7, Pd2, Pd3> Pins;

volatile static uint16_t sensorsTimer;
volatile static uint8_t PinsBuffered;
void uart_puts(const char *str);

static void FillValues()
{
  PSensor::PT pt;
  HSensor::HT ht;

  PSensor::GetValues(pt);
  HSensor::GetValues(ht);
  usRegInputBuf[0] = 'F' | ('D' << 8);
  usRegInputBuf[1] = LSensor::Read();     
  usRegInputBuf[2] = uint16_t((pt.pressure >> 16 )& 0xFFFF );
  usRegInputBuf[3] = uint16_t(pt.pressure & 0xFFFF );
  usRegInputBuf[4] = pt.temperature;
  usRegInputBuf[5] = ht.humidity;
  usRegInputBuf[6] = ht.temperature;
}


void uart_init(uint32_t baud_rate, uint32_t f_master);
uint8_t uart_getc();
void uart_putc(uint8_t data);
void uart_write(void *buf, int len);

#include <stdlib.h>
#include <string>

void cuits(int32_t value, char* buffer )
{
unsigned char p=0;
if ( value == 0 )
{
buffer[0]='0';
buffer[1]='\0';
return;
}
else if ( value < 0 )
{
buffer[p++] = '-';
value = (value<0)?-value:value;
}
for ( int s=100000; s>0; s/=10 )
{
//p++
if ( value >= s )
{
buffer[p++] = (unsigned char)(value/s) + '0';
value %= s;
}
else if ( p>0 )
{
buffer[p++] = '0';
}
}
buffer[p] = '\0';
} 
//************************************************** *****

int main()
{
//  SysClock::SetHsiDivider(SysClock::Div1); //set F_CPU to 16 MHz
  GpioA::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioB::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioC::WriteConfig<0xFF, GpioBase::In_Pullup>();
  GpioD::WriteConfig<0xFF, GpioBase::In_Pullup>();

  //T4::Timer4::Init(T4::Div_128, T4::Cfg(T4::ARPE | T4::CEN)); // 61Hz
  //T4::Timer4::EnableInterrupt();

  i2c::Init();
  LSensor::Init();
  PSensor::Init();
   
  //eMBInit(MB_RTU, DEVICE_ADDRESS, 0, 9600, MB_PAR_NONE);
  //enableInterrupts();
  //eMBEnable();

  //FillValues();
  PSensor::PT pt;
  uart_init(115200, F_CPU);
  uart_puts("\nstm8: hello, i'm kind of running, send 'R' + 'Q' to get binary sensor data\r\n");
  uint8_t pinReadTimer = 0xFF;
  uint8_t prev = 0x0; 
  while(true) {
     uint8_t tmp = uart_getc();
     if ((prev == 'R') && (tmp == 'Q')) {
       FillValues();
       uart_write(usRegInputBuf, sizeof(usRegInputBuf));
     }
    prev = tmp;
  }
}


INTERRUPT_HANDLER(Timer4_ISR, TIM4_OVR_UIF_vector - 2)
{
  T4::Timer4::ClearIntFlag();
  ++sensorsTimer;
  PinsBuffered = Pins::Read();
}
