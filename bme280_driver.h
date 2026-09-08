//
// Created by barte on 7.09.2026.
//





#ifndef BME280LIB_BME280_DRIVER_H
#define BME280LIB_BME280_DRIVER_H


#include <stdint.h>

typedef struct {
    int8_t (*ReadReg)(void *InterfacePtr, uint8_t RegisterAddress, uint8_t *Buffer, uint8_t Length);
    int8_t (*WriteReg)(void *InterfacePtr,uint8_t RegisterAddress, uint8_t NewByte);
    void (*DelayMs)(uint16_t ms);
}BME280_Driver_t;

#endif //BME280LIB_BME280_DRIVER_H