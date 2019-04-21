#pragma once
#include <iostm8.h>
#include <intrinsics.h>
#include <stdint.h>


class uart {
  uart(uint32_t baud_rate, uint32_t f_master);
  uint8_t getc();
  void putc(uint8_t ch);
  void write(void *buf, int len);
};

