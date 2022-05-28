#ifndef BON_MAIN_H
#define BON_MAIN_H

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
#define TIMER0_INTERVAL_MS        1000  //延时时间
#define TIMER1_INTERVAL_MS        1000

#define font_y 14 //字体大小（高度）
#define sub_y 58  //最下面的文字
#define cnt_y ((sub_y)-(font_y))
#define temp_x 90
#define temp_y ((cnt_y)-(font_y))
#define mois_x 70
#define mois_y ((temp_y)-(font_y))

#endif
