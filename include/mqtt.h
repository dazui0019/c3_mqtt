#ifndef BON_MQTT_H
#define BON_MQTT_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define KEEPALIVE 900

class mqtt{
    public:
        mqtt(WiFiClient& wifiClient):mqttClient(wifiClient){
              mqttClient.setKeepAlive(KEEPALIVE);
        }
        void MqttConnect();
        void MqttLoop();
        void MqttPub(String Serialmsg);
        void setLocalBroker();
        int state(){return mqttClient.state();};
        void Display(U8G2_SSD1306_128X64_NONAME_F_SW_I2C& u8g2);
    private:
        PubSubClient mqttClient;
        //服务器选择标志: 0(aliyun), 1(local server)
        uint8_t BrokerFlag = 0;
};

#endif
