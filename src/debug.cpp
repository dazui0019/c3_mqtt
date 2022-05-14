#include "debug.h"

void DebugInit(HardwareSerial& DebugSerial){
    DebugSerial.begin(115200);
}
