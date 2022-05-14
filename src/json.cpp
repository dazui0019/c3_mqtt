#include <ArduinoJson.h>
#include "debug.h"
#include "json.h"

String MQTTmessage;

//重组json数据
//InputJSON的格式应该为{"sensor":["soil","air"],"soilValue":[20, 30],"airValue":[15, 40]}
//                                               ^---soilValue对应soil ^--airValue对应air
//发送给阿里云的格式应该为{"params": {"CurrentTemperature": {"value":25},"CurrentHumidity":{"value": 57}}, {"air...": {"value":25},"SoilMoisture":{"value": 57}}}
//                                      ^~~~~标识符                          ^~~~~标识符
String InputJSONhandler(String InputStringJSON)
{
    String outputStringJSON;  //存储输出的JSON字符串。
    StaticJsonDocument<200> STM32json;    //存储从STM32接收的数据

    deserializeJson(STM32json, InputStringJSON);   //将字符串中的JSON数据反序列化到doc
    uint8_t sensorquatity = STM32json["sensor"].size();   //获取传感器种数, sensor是个数组。
    if(sensorquatity > 0){
        StaticJsonDocument<200> MQTTjson;     //存储需要发送到MQTT服务器的JSON数据
        JsonObject params = MQTTjson.createNestedObject("params"); //在MQTTjson中创建一个params对象

    //  提取传感器名称
    //  abstractSensorName(JsonArray doc["sensor"]);
    //  如果读取的是JSON数组中没有的元素，则会得到NULL
        for(uint8_t i = sensorquatity; i > 0; i--){
            const String sensor = STM32json["sensor"][i-1];//获取传感器类型(注意-1)
            if(sensor == "soil"){
            //如果数组中该位置有数据,并且等于"soil".(String 对象可以直接这么比较字符串)
                if(STM32json["soilValue"] != NULL){//检查是否有soilValue数组
                    //  从stm32发过来的数据里解析出传感器数值
                    //soilValue数组    soilValue[0]:湿度，soilValue[1]:温度
                    //根据具体的数据创建json数据对象
                    if(STM32json["soilValue"][0] != NULL){//数值不为NULL则创建该JSON对象（说明有这个数据）
                        JsonObject CurrentHumidity = params.createNestedObject("CurrentHumidity"); //在params对象中创建一个SoilMoisture对象(大括号)
                        CurrentHumidity["value"] = STM32json["soilValue"][0];   //设置对象的值{}
                    }
                    if(STM32json["soilValue"][1] != NULL){//数值不为NULL则创建该JSON对象
                        JsonObject CurrentTemperature = params.createNestedObject("CurrentTemperature"); //在params对象中创建一个SoilMoisture对象
                        CurrentTemperature["value"] = STM32json["soilValue"][1];   //设置对象的值
                    }
                }
            }else if(sensor == "air"){
                //todo
            }
        }

        serializeJson(MQTTjson, Serial); //输出到电脑串口
        serializeJson(MQTTjson, outputStringJSON);  //输出到字符串对象
        return outputStringJSON;
    }else{
        BON_DEBUG("no sensor");
        return "";
    }
}
