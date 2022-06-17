#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <ESP32_C3_TimerInterrupt.h>
#include "wlan.h"
#include "mqtt.h"
#include "frame.h"
#include "json.h"
#include "main.h"
#include "debug.h"

typedef unsigned char uint8_t;

/* 按键 */
uint8_t counter = 0;  //按键超时计数
bool timeout = false;  //按键输入超时标志位

/* 复位stm32 */
bool NoData  = true;  //0:接收到数据、1:没有接收到数据
uint8_t rst_counter = 0;
bool rstFlag = false;   //复位标志

/* 刷新连接状态 */
bool refreshState = true;

/* 温度 */
float temperature = 0.0f;
uint8_t moisture = 0;

WiFiClient wificlient;
mqtt mqttClient(wificlient);
//esp32 SCL(22) SDA(21)
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
frame stm(Serial1);

ESP32Timer ITimer0(0);
ESP32Timer ITimer1(1);
ESP32Timer ITimer2(2);

//timer0 handler IRAM_ATTR
/* 按键超时（设置服务器） */
bool IRAM_ATTR Timer0Handler(void * timerNo){
  static bool flag = false;
  if(!flag)
    if(counter < 5){
      counter ++;
    }else{
      timeout = 1;
      // ITimer0.stopTimer();  //关闭定时器
      ITimer0.attachInterruptInterval(TIMER1_INTERVAL_MS * 5000, Timer0Handler);
      flag = true;
    }
  else
    refreshState = true;
  return true;
}

//timer1 handler
/**
 * 定时器1中断处理函数
 * 有两个功能：
 * 1、检测超时（每500ms进入一次）
 * 2、超时后复位STM32（改为1000ms进入一次，用于产生STM32复位信号）
 */
bool IRAM_ATTR Timer1Handler(void * timerNo){
  static bool flag = 0; //切换引脚电平

  if(NoData)
    rst_counter++;  //每次接收到数据，rst_counter都会被置零，只有20s没有接收到才会加到>20
  else{
    NoData = 1;
    rst_counter = 0;
  }

  if(rst_counter > 20){
    if(!flag){  //若flag = 0，则将引脚拉低
      //timer interrupt toggles pin LED_BUILTIN
      digitalWrite(LED0, LOW);//将复位引脚拉低（跟LED0是同一个引脚）
      //将定时器定时时间改为500ms（原来是1000ms，用来复位可能太长了）
      ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 500, Timer1Handler);
      flag = 1; //置一，下一次进入中断
    }else{  //flag == 1，将引脚拉高
      flag = 0; //重新置零
      digitalWrite(LED0, HIGH);
      ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, Timer1Handler);  //重新改为1000ms
      rst_counter = 0;  //清除计数
    }
  }
  return true;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON, INPUT);
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);

  Serial.begin(115200);   //用来调试，输出到电脑
  Serial1.begin(115200);  //用来和stm32通信
  Serial1.setPins(1, 0);

  while (!Serial && !Serial1){}

  u8g2.begin();
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.setFont(u8g2_font_helvR10_tf);	// choose a suitable font
  u8g2.drawStr(20, 25, "Configure WiFi");
  u8g2.sendBuffer();
  WiFiManagerConnect();
  u8g2.clearBuffer();
  waitForInput();//等待输入
  mqttClient.MqttConnect();
  // mqttClient.Display(u8g2); //显示mqtt 连接状态
  layout_display(); //显示框架
  ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, Timer1Handler);
}

void loop() {
  // put your main code here, to run repeatedly:
  mqttClient.MqttLoop();
  MonitoSerialPort(); //监听串口
  if(refreshState){
    stateDisplay();
    refreshState = 0;
  }
}

/* 监听串口 */
void MonitoSerialPort()
{

  String StmMessage;    //存储来自STM32的数据, Serial2
  String DebugMessage;  //存储来自debug口的数据, Serial(不是1)

  while (Serial.available() > 0){         //如果缓冲区有数据则循环读取
    DebugMessage += (char)Serial.read();  //读取缓冲区的数据放进serialMessage，只能一字节一字节读取
    delay(2);                             //延时一下，应为读取缓冲会比接收快
  }

  // 接收来自STM32的数据
  while (Serial1.available() > 0){
    StmMessage += (char)Serial1.read();
    delay(2);
  }

  //解析来自串口的数据
  if (StmMessage.length() > 0){  //如果字符串长度大于零，说明接收到数据了。
    mqttClient.MqttPub(StmMessage); //发布
    dataDisplay();
    NoData = 0;
  }
  else if(DebugMessage.length() > 0){
    if(DebugMessage == "get"){
      stm.get_data();
      BON_DEBUG(DebugMessage);
    }
    //todo 其他口
  }
}

//等待输入
void waitForInput()
{
  //显示提示信息
  u8g2.drawStr(0, 10, "press the button:");
  u8g2.drawStr(20, 25, "local broker!");
  u8g2.sendBuffer();
  delay(500);

  timeout = 0;
  ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, Timer0Handler);
  while (!timeout)  //超时退出
  {
    u8g2.setCursor(0, 40);
    u8g2.printf("%d", counter);
    u8g2.sendBuffer();
    if((digitalRead(BUTTON) == LOW)){  //按下时，引脚置零
      delay(10);  //消抖
      if(digitalRead(BUTTON) == LOW){
        // ticker.detach();
        mqttClient.setLocalBroker();  //设置为本地服务器
        u8g2.clear();
        u8g2.drawStr(0, 40, "set aliyun");
        u8g2.sendBuffer();
        return;
      }
    }
  }
  u8g2.clear();
  u8g2.drawStr(0, 40, "set local broker");
  u8g2.sendBuffer();
}

//显示布局
void layout_display(){
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_7x14_tf);	// choose a suitable font
  u8g2.drawStr(0, mois_y, "moisture: ");
  u8g2.drawStr(0, temp_y, "temperature: ");
  // u8g2.drawStr(0, cnt_y, "cnt: emqx");
  // u8g2.drawStr(0,sub_y,"sub: /esp32/post");	// write something to the internal memory
  u8g2.drawStr(0, cnt_y, "cnt: ");
  u8g2.drawStr(0,sub_y,"sub: ");	// write something to the internal memory
  u8g2.drawStr(0, wifi_y, "wifi: ");
  u8g2.sendBuffer();
}

//在已显示的布局上显示数据
void dataDisplay(){
  /* 如果有0的话下一次显示会不正常，所以清空一下 */
  u8g2.setCursor(mois_x, mois_y);
  u8g2.print("  ");
  u8g2.setCursor(temp_x, temp_y);
  u8g2.print("   ");
  u8g2.sendBuffer();
  /* 显示数据 */
  u8g2.setCursor(mois_x, mois_y);
  u8g2.printf("%2d", moisture);
  u8g2.setCursor(temp_x, temp_y);
  u8g2.printf("%3.1f", temperature);
  u8g2.sendBuffer();
}

/* 显示wifi等的连接状态 */
void stateDisplay(){
  static int a = 0;
  if(WiFi.status() == WL_CONNECTED){
    u8g2.setCursor(wifi_x, wifi_y);
    u8g2.printf("%s", WiFi.SSID());
    if(!mqttClient.state()){
      u8g2.setCursor(cnt_x, cnt_y);
      if(mqttClient.getBroker()){
        u8g2.print("emqx");
      }else{
        u8g2.print("aliyun");
      }
    }else{
      u8g2.setCursor(cnt_x, cnt_y);
      u8g2.printf("%d", mqttClient.state());
    }
  }
  u8g2.sendBuffer();
}
