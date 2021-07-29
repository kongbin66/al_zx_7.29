#include "config.h"
/*---------------------------连接ONENET平台-------------------------------*/
bool onenet_connect()

{
  //连接OneNet并上传数据
  Serial.print("connecting to OneNet IOT...");
  client.setServer(mqtt_server, port);                   //设置客户端连接的服务器,连接Onenet服务器, 使用6002端口
  client.connect(mqtt_devid, mqtt_pubid, mqtt_password); //客户端连接到指定的产品的指定设备.同时输入鉴权信息
  if(client.connected()) return true;
  else return false;
}

/*******发送温度湿度时间戳经纬度到onenet*****
发送温度湿度经纬度时间戳到ONENET
订阅主题："/thing/property/post"
发送成功返回：true
******************************************/
bool sendTempAndHumi()
{
 
  if (client.connected())
  {
    //先拼接出json字符串
    char param[1024];
    char jsonBuf[1024];

    if (current_rec_State == START_RECING)
      start_time = tmsp, last_time = tmsp;
    else
      last_time = tmsp;
    #ifdef GSM1
    sprintf(param, "{\"temp\":{\"value\":%.2f,\"time\":%lu000},\"humi\":{\"value\":%.2f,\"time\":%lu000},\"le\":{\"value\":%.4f,\"time\":%lu000},\"ln\":{\"value\":%.4f,\"time\":%lu000},\"start_time\":{\"value\":%lu,\"time\":%lu000},\"last_time\":{\"value\":%lu,\"time\":%lu000},\"tempLA\":{\"value\":%ld,\"time\":%lu000},\"tempUA\":{\"value\":%ld,\"time\":%lu000}}", tp, tmsp, h, tmsp, e, tmsp, n, tmsp, start_time, tmsp, last_time, tmsp, tempLA, tmsp, tempUA, tmsp);
    //sprintf(param, "{\"temp\":{\"value\":%.2f,\"time\":%u000},\"humi\":{\"value\":%.2f,\"time\":%u000},\"le\":{\"value\":%.4f,\"time\":%u000},\"ln\":{\"value\":%.4f,\"time\":%u000},\"start_time\":{\"value\":%u,\"time\":%u000},\"last_time\":{\"value\":%u,\"time\":%u000}}", tp, tmsp, h, tmsp, e, tmsp, n, tmsp, start_time, tmsp, last_time, tmsp);
    #endif
    
    #ifdef GSM2
    sprintf(param, "{\"temp\":{\"value\":%.2f,\"time\":%lu000},\"humi\":{\"value\":%.2f,\"time\":%lu000},\"le\":{\"value\":%.4f,\"time\":%lu000},\"ln\":{\"value\":%.4f,\"time\":%lu000},\"start_time\":{\"value\":%lu,\"time\":%lu000},\"last_time\":{\"value\":%lu,\"time\":%lu000},\"tempLA\":{\"value\":%ld,\"time\":%lu000},\"tempUA\":{\"value\":%ld,\"time\":%lu000}}", tp, tmsp, h, tmsp, LAC*1.0, tmsp, CIID*1.0, tmsp, start_time, tmsp, last_time, tmsp, tempLA, tmsp, tempUA, tmsp);
    #endif
    sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, param);
    //再从mqtt客户端中发布post消息
    if (client.publish(ONENET_TOPIC_PROP_POST, jsonBuf))
    {
      Serial.print("Post message to cloud: ");
      Serial.println(jsonBuf);
      current_rec_State = KEEP_RECING;
      //OneNet_connected_Time = millis();
      //rec_count++;
      return true;
    }
    else
    {
      Serial.println("Publish message to cloud failed!");
      return false;
    }
  }
  else
    Serial.println("connect failed!");

  return false;
}


void getDesired()
{
   Serial.printf("  in getDesired !\n");
  //1.检测onenet连接
  if (client.connected())
  {
    Serial.printf("    in getDesired ok!\n");
    client.subscribe(ONENET_TOPIC_DESIRED_GET);    //订阅获得期望值请求主题
    client.subscribe(ONENET_TOPIC_DESIRED_GET_RE); //订阅设备期望值获取响应主题
    client.setCallback(callback);                  //设置期望值回调
    //先拼接出json字符串
    char param[164];
    char jsonBuf[356];
    sprintf(param, "[\"temp_alarm\",\"tempL\",\"tempU\",\"period\"]"); //我们把要上传的数据写在param里
    //postMsgId += 1;
    sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, param);
    //再从mqtt客户端中发布post消息
    if (client.publish(ONENET_TOPIC_DESIRED_GET, jsonBuf))
    {
      Serial.print("Post message2 to cloud: ");
      Serial.println(jsonBuf);
      f_MQTT_ON = 1;
    }
    else
    {
      Serial.println("Publish message to cloud failed!");
    }
   
  }
  Serial.printf("  out getDesired !\n");
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk");
  Serial.println("message rev:");
  Serial.println(topic);
  for (size_t i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if (strstr(topic, ONENET_TOPIC_DESIRED_GET))
  {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.println("parse json failed");
      return;
    }
    JsonObject setAlinkMsgObj = doc.as<JsonObject>();
    temp_alarm = setAlinkMsgObj["data"]["temp_alarm"]["value"];
    tempU = setAlinkMsgObj["data"]["tempU"]["value"];
    tempL = setAlinkMsgObj["data"]["tempL"]["value"];
    period = ((int)setAlinkMsgObj["data"]["period"]["value"]) * 60;
    eeprom_config_set(temp_alarm, (uint32_t)period, tempU, tempL);
    

    if (temp_alarm)
    {
      if (tp > tempU)tempUA++;
      else tempUA = 0;
      if (tp < tempL)tempLA++;
      else tempLA = 0;
      Serial.printf("temp_alarm:%d,tempUA:%lu,tempLA:%lu\n", temp_alarm, tempUA, tempLA);
    }

    if (sendTempAndHumi()) //数据发送成功
    {
      Serial.printf("sendTempAndHumi ok!\n");
      saveDataToCSV("/list.csv");
      postMsgId++; //记录到文件系统
    }
    else Serial.printf("sendTempAndHumi false!\n");
    f_MQTT_ON = 0;
  }
}