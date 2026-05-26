#pragma once

#include <vector>
#include <span>
#include "result.hpp"
#include "baos.hpp"
#include "uart.cpp"
using namespace result;

namespace baos {
    uint8_t highByte(uint16_t value){
        return (value >> 8) & 0xff;
    }
    uint8_t lowByte(uint16_t value){
        return value & 0xff;
    }
    const uint8_t knx_baos_cmd_reset[] = { 0x10, 0x40, 0x40, 0x16 };
    const uint8_t knx_baos_cmd_init1[] = { 0xA7 };
    const uint8_t knx_baos_cmd_init2[] = { 0xFC, 0x00, 0x08, 0x01, 0x40, 0x10, 0x01 };
    const uint8_t knx_baos_cmd_init3[] = { 0xFC, 0x00, 0x08, 0x01, 0xC9, 0x10, 0x01 };
    const uint8_t knx_baos_cmd_init4_layer[] = {  0xF6, 0x00, 0x08, 0x01, 0x34, 0x10, 0x01, 0x00 };
    const uint8_t knx_baos_cmd_init4_baos[] = { 0xF6, 0x00, 0x08, 0x01, 0x34, 0x10, 0x01, 0xF0 };
    const uint8_t knx_baos_cmd_init5[] = {0xFC, 0x00, 0x08, 0x01, 0x34, 0x10, 0x01 };
    const uint8_t knx_baos_cmd_init6[] = {  0xFC, 0x00, 0x08, 0x01, 0x33, 0x10, 0x01 };
    const uint8_t knx_baos_cmd_init7[] = { 0xFC, 0x00, 0x00, 0x01, 0x38, 0x10, 0x01 };
    #define lengthBufforCMD 100
    static uint8_t bufforCMD[lengthBufforCMD];
    static uint8_t lastParity = 0x53;

    class BaosHandler{
        private:
            uart::Uart baos_uart = uart::Uart::init(UART_NUM_1);
        public:
            void sendRaw(uint8_t* data, uint8_t length) {
                baos_uart.write(std::span<uint8_t>(data, length));
                delay(3);
            }
            void sendCmd(uint8_t* data, uint8_t length) {
                baos_uart.write(std::span<uint8_t>(data, length));
                delay(3);
                baos_uart.write(std::span<uint8_t>(0xE5, 1));
                delay(40);
            }
            void sendBaosCmd(uint8_t* data, uint8_t length) {
                //Czyszczenie bufora
                memset(bufforCMD, 0, lengthBufforCMD);
                  //Ustawienie bitu ramki baos CR
                  if (lastParity == 0x73) {
                  lastParity = 0x53;
                }
                else {
                  lastParity = 0x73;
                }
                bufforCMD[0] = 0x68;
                bufforCMD[1] = length + 1;
                bufforCMD[2] = length + 1;
                bufforCMD[3] = 0x68;
                bufforCMD[4] = lastParity;
                //skopiowanie danych
                for (uint8_t i = 0; i < length; i++) {
                  bufforCMD[5 + i] = data[i];
                }
                //suma kontrolna
                bufforCMD[5 + length] = calculateC(&bufforCMD[4], length + 1);
                bufforCMD[5 + length + 1] = 0x16;
            
                sendCmd(bufforCMD, 5 + length + 1 + 1);
            }               
            uint8_t calculateC(uint8_t* data, uint8_t len) {
              uint16_t sum = 0;
              for (uint8_t i = 0; i < len; i++) {
                sum += data[i];
              }
              return (uint8_t)((uint16_t)sum % 256);
            }
            void setDataPointBinary(uint16_t dataPointIndex, uint8_t binary) {
              uint8_t telegramBinary[]={0xF0,0x06,highByte(dataPointIndex),lowByte(dataPointIndex),0x0,0x1,highByte(dataPointIndex),lowByte(dataPointIndex),0x03,0x01,binary};
               sendBaosCmd(telegramBinary, sizeof(telegramBinary));
            }
            void setDataPoint2Byte(uint16_t dataPointIndex, uint16_t value) {
              uint8_t telegram[] = {
                0xF0, 0x06,
                highByte(dataPointIndex), lowByte(dataPointIndex),
                0x00, 0x01,
                highByte(dataPointIndex), lowByte(dataPointIndex),
                0x03,
                0x02,
                highByte(value), lowByte(value)
              };

              sendBaosCmd(telegram, sizeof(telegram));
            }

            void getDataPointValue(uint16_t dataPointIndex,uint16_t maxDataPoints,uint8_t filter){
              uint8_t telegram[] = {
                0xF0, 0x05,
                highByte(dataPointIndex), lowByte(dataPointIndex),
                highByte(maxDataPoints), lowByte(maxDataPoints),
                filter
              };
            }
            void knxBaos_initLayer_Link() {
                knxBaos_sendRaw((uint8_t*)knx_baos_cmd_reset, sizeof(knx_baos_cmd_reset));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init1, sizeof(knx_baos_cmd_init1));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init2, sizeof(knx_baos_cmd_init2));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init3, sizeof(knx_baos_cmd_init3));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init4_layer, sizeof(knx_baos_cmd_init4_layer));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init5, sizeof(knx_baos_cmd_init5));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init6, sizeof(knx_baos_cmd_init6));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init7, sizeof(knx_baos_cmd_init7));
            }
            
            void knxBaos_initLayer_Baos() {
                knxBaos_sendRaw((uint8_t*)knx_baos_cmd_reset, sizeof(knx_baos_cmd_reset));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init1, sizeof(knx_baos_cmd_init1));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init2, sizeof(knx_baos_cmd_init2));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init3, sizeof(knx_baos_cmd_init3));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init4_baos, sizeof(knx_baos_cmd_init4_baos));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init5, sizeof(knx_baos_cmd_init5));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init6, sizeof(knx_baos_cmd_init6));
                knxBaos_sendBaosCmd((uint8_t*)knx_baos_cmd_init7, sizeof(knx_baos_cmd_init7));
            }
    };
            
}
