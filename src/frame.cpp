/**
 * maximum size: 4 byte
 * start byte: 0xAA
 * device ID:
 * function code:
 * stop byte: 0x55
 *
 * e.g. 0xAA 0x4C 0x4F 0x55
 */

#include <Arduino.h>
#include "frame.h"

void frame::led_on(void){
    frame_send(on);
}

void frame::led_off(void){
    frame_send(off);
}

void frame::get_data(void){
    frame_send(get);
}

void frame::frame_send(const uint8_t message[]){
    for(int i = 0; i < 4; i++){
        Serial.write(message[i]);  //单字节发送
    }
}


