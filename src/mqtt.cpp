#include <Arduino.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include "main.h"
#include "mqtt.h"
#include "json.h"
#include "debug.h"

//阿里云mqtt服务器地址及端口
const char aliBrokerAddress[] = "a15JvS2y9ut.iot-as-mqtt.cn-shanghai.aliyuncs.com";
const int BrokerPort = 1883;
//用户名密码等
const char UserName[]     = "hicode_soil&a15JvS2y9ut";
const char mqttClientID[] = "a15JvS2y9ut.hicode_soil|securemode=2,signmethod=hmacsha256,timestamp=1652252328359|";
const char mqttPassword[] = "18c74ab51c0802745098bbe50952dd7144cfb590d8166d0ee8289e475f4c5c24";
//Topic
const char Topic[] = "/sys/a15JvS2y9ut/hicode_soil/thing/event/property/post";//需要往这个topic发送(阿里云)
const char emqxTopic[] = "/esp32/post"; //本地broker

//本地服务器
const char localBrokerAddress[] = "192.168.2.148";
const char localClientID[]    = "esp32";

void mqtt::setLocalBroker(){
  BrokerFlag = 0;
}

//连接MQTT服务器
void mqtt::MqttConnect(void)
{
  if(!BrokerFlag){
    mqttClient.setServer(aliBrokerAddress, BrokerPort);  //设置阿里云服务器
    if (mqttClient.connect(mqttClientID, UserName, mqttPassword)){  //这个就不用循环卡在这里连接了。
      BON_DEBUG_LN("MQTT Server Connected");
    } else {
      BON_DEBUG("Failed! Error Code:");
      BON_DEBUG_LN(mqttClient.state()); //输出mqtt连接状态
      delay(100);
    }
  }
  else if(BrokerFlag){
    mqttClient.setServer(localBrokerAddress, BrokerPort);  //设置本地服务器
    if (mqttClient.connect(localClientID)){  //这个就不用循环卡在这里连接了。
      BON_DEBUG_LN("MQTT Server Connected");
    } else {
      BON_DEBUG("Failed! Error Code:");
      BON_DEBUG_LN(mqttClient.state()); //输出mqtt连接状态
      delay(100);
    }
  }
}

//要放在主循环
void mqtt::MqttLoop(void)
{
  if (mqttClient.connected())
    mqttClient.loop(); //mqttclient.loop()应该是用来检测订阅的主题是否有数据接收的
  else
    MqttConnect(); //如果不是已连接状态，则重新连接。
}

//MQTT报文发送函数
void mqtt::MqttPub(String Serialmsg)
{
//  需要将Serialmsg重组（STM32先构建一个能让ESP32使用的Json格式，然后由Json构建成ALink格式）
  String outputJOSN = InputJSONhandler(Serialmsg);
  char Pubmsg[outputJOSN.length() + 1];//因为字符串数组需要多一个位置存放'/0',所以需要将长度+1
  strcpy(Pubmsg, outputJOSN.c_str());  //把字符对象中的字符串转成字符，并放入字符串数组中(publish无法使用String类型)
  if(!BrokerFlag)
    mqttClient.publish(Topic, Pubmsg);
  else if(BrokerFlag)
    mqttClient.publish(emqxTopic, Pubmsg);
}

//在oled上显示连接信息
void mqtt::Display(U8G2_SSD1306_128X64_NONAME_F_SW_I2C& u8g2){
  //todo display state
  u8g2.clear();
  u8g2.setCursor(0, 30);
  if(!state()){
    switch(BrokerFlag){
      case 0:
        u8g2.print("connected to aliyun");
        break;
      case 1:
        u8g2.print("connected to emqx");
        break;
      default:
        break;
    }
    u8g2.sendBuffer();
  }else{
    u8g2.print("Failed! Error Code:");
    u8g2.setCursor(35, 50);
    u8g2.printf("%d", state());
    u8g2.sendBuffer();
  }
}
