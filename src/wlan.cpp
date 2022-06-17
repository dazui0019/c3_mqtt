#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFiManager.h>
#include "wlan.h"
#include "debug.h"

const char WiFiSSID[]   = "test";
const char PASSWORD []  = "dazui12137";

//WiFi连接函数
void WiFiConnect(void){
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WiFiSSID, PASSWORD);
  Serial.print("wifi connecting");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.printf("WiFi Connected to %s", WiFi.SSID());
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiManagerConnect(){
    // 建立WiFiManager对象
    WiFiManager wifiManager;
    bool res; //连接状态的标志

    // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称
    // wifiManager.resetSettings();    //清除WiFi信息
    wifiManager.setTimeout(30);
    Serial.print("timeout: 30s");
    res = wifiManager.autoConnect("AutoConnectAP");
    if(!res){ //若res为0则说明超时, 采用基本的连接函数
      Serial.println("Timeout");
      wifiManager.disconnect();
      WiFiConnect();
    }else{
      // WiFi连接成功后将通过串口监视器输出连接成功信息
      Serial.println("");
      Serial.printf("Connected to %s", WiFi.SSID());
      Serial.println("");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    }
}
