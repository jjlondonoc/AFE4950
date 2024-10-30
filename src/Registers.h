#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <Arduino.h>

struct Register {
    uint8_t address;
    uint32_t data;
};

extern Register page_1_registers[];
extern size_t page_1_registers_size;

extern Register page_0_registers[];
extern size_t page_0_registers_size;

#endif // __REGISTERS_H__