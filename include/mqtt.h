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
        uint8_t getBroker(){return BrokerFlag;};
    private:
        PubSubClient mqttClient;
        //服务器选择标志: 0(aliyun), 1(local server)
        uint8_t BrokerFlag = 1;
        //连接状态
        uint8_t ConnectState = 0;
};

#endif
