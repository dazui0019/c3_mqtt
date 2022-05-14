#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include "wlan.h"

const char WiFiSSID[]   = "test";
const char PASSWORD []  = "dazui12137";

//WiFi连接函数
void WiFiConnect(void){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFiSSID, PASSWORD);
  Serial.print("wifi connecting");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}
