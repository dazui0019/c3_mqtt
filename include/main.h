#ifndef BON_MAIN_H
#define BON_MAIN_H

void MonitoSerialPort();
void waitForInput();
void layout_display();
void dataDisplay();
void stateDisplay();

typedef unsigned char uint8_t;

// IIC OLED
#define SCL 5
#define SDA 4

// LED
#define LED0 12
#define LED1 13

// button
#define BUTTON 9

//定义定时器延时时间
#define TIMER0_INTERVAL_MS        1000  //延时时间（us）
#define TIMER1_INTERVAL_MS        1000
#define TIMER2_INTERVAL_MS        1000

/* 显示坐标 */
#define font_H 10 //字体大小（高度）
#define line_H ((font_H)+(2))   //行高（4表示每行间隔4个像素点）

#define temp_x 90
#define temp_y ((mois_y)-(line_H))

#define mois_x 70
#define mois_y ((wifi_y)-(line_H))

#define wifi_x 40
#define wifi_y ((cnt_y)-(line_H))

#define cnt_x 35
#define cnt_y ((sub_y)-(line_H))

#define sub_x 30
#define sub_y 64  //订阅的主题（显示在最下面，字的坐标是右下角为原点）

#endif
