#pragma once

#include <iostm8.h>
#include <intrinsics.h>
#include <stdint.h>

namespace Mcudrv {
class uart {
  void init(uint32_t baud_rate, uint32_t f_master);
  uint8_t getc();
  void putc(uint8_t ch);
  void write(void *buf, int len);
};


uart::init(uint32_t baud_rate, uint32_t f_master)
{
  unsigned long brr;
   
  PD_DDR_bit.DDR5 = 1;  //TX
  PD_DDR_bit.DDR6 = 0;  //RX
   
  PD_CR1_bit.C16 = 0;
  PD_CR2_bit.C26 = 0;
   
  brr = f_master/baud_rate;
     
  UART1_BRR2 = brr & 0x000F;
  UART1_BRR2 |= brr >> 12;
  UART1_BRR1 = (brr >> 4) & 0x00FF;
   
  UART1_CR1_PIEN = 0;
  UART1_CR1_PCEN = 0;
  UART1_CR1_M = 0;
  UART1_CR1_UART0 = 0;
   
  UART1_CR2_TIEN = 0;
  UART1_CR2_TCIEN = 0;
  UART1_CR2_RIEN = 0;
  UART1_CR2_ILIEN = 0;
  UART1_CR2_TEN = 1;
  UART1_CR2_REN = 1;
  UART1_CR2_SBK = 0;   
  UART1_CR3_STOP = 0;
}

uint8_t uart::getc()
{
   uint8_t data;
       
   while(!UART1_SR_RXNE);
   data = UART1_DR;
   return data;
}


void uart::putc(uint8_t data)
{
    while(!UART1_SR_TXE);
    UART1_DR = data; 
}

void uart::write(void *buf, int len)
{
  uint8_t *ptr = (uint8_t *) buf;
  while(len--)
    uart::putc(*ptr++);
}


}