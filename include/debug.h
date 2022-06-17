/**
 *
 * 使用debug前，先把Serial初始化好
 *
 */

#ifndef BON_DEBUG_H
#define BON_DEBUG_H

#include <Arduino.h>

//指定debug口
#define DEBUG_SER Serial

//debug 开关
#define BON_DEBUG_ON    false

//c语言 预编译器 (#)运算符、(##)粘合剂、(...)变参宏
#define BON_INFO(fmt,arg...)        DEBUG_SER.printf("<<-INFO->> "#fmt"\r\n",##arg)
#define BON_ERROR(fmt,arg...)       DEBUG_SER.printf("<<-ERROR->> "#fmt"\r\n",##arg)
#define BON_DEBUG_LN(fmt,arg...)       do{\
                                    if(BON_DEBUG_ON)\
                                    DEBUG_SER.printf("<<-DEBUG->> [%s: %d]"#fmt"\r\n",__func__,__LINE__, ##arg);\
                                    }while(0)
#define BON_DEBUG(fmt,arg...)       do{\
                                    if(BON_DEBUG_ON)\
                                    DEBUG_SER.printf("<<-DEBUG->> [%s: %d]"#fmt,__func__,__LINE__, ##arg);\
                                    }while(0)

#endif
