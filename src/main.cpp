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

uint8_t counter = 0;
uint8_t timeout = 0;
uint8_t NoData  = 1;  //0:接收到数据、1:没有接收到数据
uint8_t rst_counter = 0;
uint8_t rstFlag = 0;

float temperature = 0.0;
uint8_t moisture = 0;

WiFiClient wificlient;
mqtt mqttClient(wificlient);
//esp32 SCL(22) SDA(21)
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
frame stm(Serial1);
void MonitoSerialPort();
void waitForInput();
void layout_display();
void dataDisplay();

ESP32Timer ITimer0(0);
ESP32Timer ITimer1(1);
ESP32Timer ITimer2(2);

//timer0 handler IRAM_ATTR
bool setFlag(void * timerNo){
  if(counter < 5){
    counter ++;
  }else{
    timeout = 1;
    ITimer0.stopTimer();
  }
  return true;
}

//timer1 handler
bool IRAM_ATTR Timer1Handler(void * timerNo)
{
  static bool flag = 0;

  if(NoData)
    rst_counter++;
  else{
    NoData = 1;
    rst_counter = 0;
  }
  if(rst_counter > 20){
    if(!flag){
      //timer interrupt toggles pin LED_BUILTIN
      digitalWrite(LED0, LOW);
      ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 500, Timer1Handler);
      flag = 1;
    }else{
      flag = 0;
      digitalWrite(LED0, HIGH);
      ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, Timer1Handler);
      rst_counter = 0;
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
  Serial1.setPins(0, 1);

  while (!Serial && !Serial1){}

  u8g2.begin();
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.setFont(u8g2_font_helvR10_tf);	// choose a suitable font
  WiFiConnect();
  waitForInput();//等待输入
  mqttClient.MqttConnect();
  delay(1000);
  mqttClient.Display(u8g2); //显示mqtt 连接状态
  layout_display();
  ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, Timer1Handler);
}

void loop() {
  // put your main code here, to run repeatedly:
  mqttClient.MqttLoop();
  MonitoSerialPort();
}

//监听串口
void MonitoSerialPort()
{

  String StmMessage;//存储来自STM32的数据, Serial2
  String DebugMessage;//存储来自debug口的数据, Serial(不是1)

  while (Serial.available() > 0){           //如果缓冲区有数据则循环读取
    DebugMessage += (char)Serial.read();  //读取缓冲区的数据放进serialMessage，只能一字节一字节读取
    delay(2);                              //延时一下，应为读取缓冲会比接收快
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
    //todo
  }
}

//等待输入
void waitForInput()
{
  //显示提示信息
  u8g2.drawStr(0, 10, "press the button:");
  u8g2.drawStr(0, 20, "local broker!");
  u8g2.sendBuffer();
  delay(500);

  timeout = 0;
  ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, setFlag);
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

void layout_display(){
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_7x14_tf);	// choose a suitable font
  u8g2.drawStr(0, mois_y, "moisture: ");
  u8g2.drawStr(0, temp_y, "temperature: ");
  u8g2.drawStr(0, cnt_y, "cnt: emqx");
  u8g2.drawStr(0,sub_y,"sub: /esp32/post");	// write something to the internal memory
  u8g2.sendBuffer();
}

void dataDisplay(){
  /* 如果有0的话下一次显示会不正常，所以清空一下 */
  u8g2.setCursor(mois_x, mois_y);
  u8g2.print("  ");
  u8g2.setCursor(temp_x, temp_y);
  u8g2.printf("   ");
  u8g2.sendBuffer();
  /* 显示数据 */
  u8g2.setCursor(mois_x, mois_y);
  u8g2.printf("%2d", moisture);
  u8g2.setCursor(temp_x, temp_y);
  u8g2.printf("%3.1f", temperature);
  u8g2.sendBuffer();
}
