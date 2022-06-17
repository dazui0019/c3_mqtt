#ifndef BON_FRAME_H
#define BON_FRAME_H

#include "main.h"
#include <Arduino.h>

#define FRAME_LENGTH 4

class frame{
    public:
        frame(HardwareSerial& frameSerial): serial(frameSerial){//C++新增的初始化方式 i(1) 相当于 i = 1;
            serial.begin(115200);
            serial.setPins(1, 0);
            while (!serial){}
        }
        void led_on();
        void led_off();
        void get_data(void);
        HardwareSerial serial;
    private:
        void frame_send(const uint8_t message[]);
        uint8_t reTry = 0;  //重连计数
        const uint8_t on[FRAME_LENGTH]   = {0xaa, 0x4c, 0x4f, 0x55};
        const uint8_t off[FRAME_LENGTH]  = {0xaa, 0x4c, 0x46, 0x55};
        const uint8_t get[FRAME_LENGTH] = {0xaa, 0x53, 0x55, 0x55};
};

#endif
