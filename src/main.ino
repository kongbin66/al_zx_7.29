#include "config.h"

int rollback = 0;

/**************************************************************
//第二核创建按键任务
***************************************************************/
void codeForTask1(void *parameter)
{

  while (1) //这是核1 的loop
  {
    vTaskDelay(10);
    button.tick(); //扫描按键
    button2.tick();
    // Serial.printf("k\n");
  }
  vTaskDelete(NULL);
}
/*************************************************************
//ds1302任务
**************************************************************/
void ds1302_task(void *parameter)
{
  uint8_t sec = 0;
  while (1)
  {
    ds_rtc.getDateTime(&now1); //读取时间参数到NOW
    now_unixtime = unixtime();
    if (now1.second == sec + 1)
    {
      sys_sec++;
      // Serial.printf("sec:%d\n",sys_sec);
    }
    sec = now1.second;
    vTaskDelay(100);
    //Serial.printf("d\n");
  }
  vTaskDelete(NULL);
}
/*****************************************************************
通讯协议任务
*****************************************************************/
void xieyi_Task(void *parameter)
{
  while (1) //这是核1 的loop
  {
    xieyi_scan();
    vTaskDelay(200);
    //Serial.printf("x\n");
  }
  vTaskDelete(NULL);
}

/*****************************************************************************************************************/

void setup()
{
  gpio_hold_dis(GPIO_NUM_32); //解锁电源引脚
  gpio_deep_sleep_hold_dis();

  hardware_init();            //硬件初始化
  get_eeprom_firstBootFlag(); //获取EEPROM第1位,判断是否是初次开机
  eeprom_config_init();       //初始化EEPROM
  software_init();            //软件初始化

  xTaskCreatePinnedToCore(xieyi_Task, "xieyi_Task", 2000, NULL, 2, &xieyi_task, tskNO_AFFINITY); //创建DS1302任务
  xTaskCreatePinnedToCore(ds1302_task, "ds1302_task", 1000, NULL, 2, &ds_task, tskNO_AFFINITY);  //创建DS1302任务
  xTaskCreatePinnedToCore(codeForTask1, "task1", 3000, NULL, 2, &task1, tskNO_AFFINITY);
  if (rollback)
  {
    /*************如果rollback置1, 会恢复出厂设置,数据全清***********/
    Serial.println("clean EEPROM");
    EEPROM.write(1, 0);
    EEPROM.commit();
    Serial.println("OK");
    ESP.deepSleep(300000000);
  }
  else
  {
    alFFS_init(); //初始化FFS
    wakeup_init_time();
  }

  if (oledState == OLED_ON)
  {
    showWelcome();
    //showsim800init_Screen();
    //modem.begin();
    //modem.sleepEnable(0);
    screen_loopEnabled = true;
    postMsgId = 0; //清记录条数
  }
  else
  {
    if (workingState == WORKING && (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)) //不是开机，是定时唤醒。
    {
      //showBFLC_Screen();
      send_Msg_var_GSM_while_OLED_on(0);
      while (f_MQTT_ON)
      {
        client.loop();
      }
      go_sleep_a_while_with_ext0(); //休眠
    }
  }
}


// void TEST()
// {
//     char *p= "538e";
//     char *str;

//     int h=strtol(p,&str,16);
//     //Serial.write(h);
//     Serial.println(h);
// }



void loop()
{
  // TEST();



  if(!Battery_test()) dianliangloop();
  if (oledState == OLED_ON)
  {
    getTempAndHumilocation();
    if (temp_alarm)
    {
      if ((tp > tempU) || (tp < tempL))
        digitalWrite(LED, LOW);
      else
        digitalWrite(LED, HIGH);
    }
    screen_loop(); //展示和滚屏
    key_loop();
    screen_show();                               //OLED最终显示
    if (now_unixtime - last_rec_stamp >= period&&!f_MQTT_ON) //记录间隔到了吗？
    {
      send_Msg_var_GSM_while_OLED_on(1);
    }
  }
  //客户端扫描

  if (f_MQTT_ON)
  {
    Serial.printf("f_MQTT_ON=%d\n", f_MQTT_ON);
    client.loop();
  }
  else
    oled_on_off_switch();
}

/*****MQTT设置订阅主题,并设定期望值回调函数*****/
void set_topcial()
{
  Serial.printf("in set_topcial\n");
  if (client.connected())
  {
    Serial.printf("in set_topcial ok!\n");
    char subscribeTopic[75];
    char topicTemplate[] = "$sys/%s/%s/cmd/request/#"; //信息模板
    snprintf(subscribeTopic, 75, topicTemplate, mqtt_pubid, mqtt_devid);
    client.subscribe(subscribeTopic); //订阅命令下发主题
    getDesired();                     //获得云端数据
  }
  Serial.printf("out set_topcial !\n");
}

/******************************************************************************
漏发处理
根据上次发送信息状态，更新数据发送一条数据
*******************************************************************************/
bool jiexi_lose()
{
  Serial.print("in jiexi_lose\n");
  static bool f_last_send_ok = 1; //上次漏发上传成功标志
  Serial.printf("lose_count=%d,f_send_ok=%d,bool_send1_ok=%d\n", lose_count, f_send_ok, f_last_send_ok);
  if (client.connected())
  {
    if (f_last_send_ok) //上一次发送成功，重新读取lose
    {
      getADataFromCSV("/lose.csv");
      parseData(&tmsp, &tp, &h, &e, &n);
    }
    Serial.printf("in jiexi_lose.tmsp=%ld,tp=%.2f,h=%.2f,e=%.4f,n=%.4f\n", tmsp, tp, h, e, n);

    if (sendTempAndHumi())
    {
      f_send_ok++;
      f_last_send_ok = 1;
       saveDataToCSV("/list.csv");
       postMsgId++; //记录到文件系统
      return true;
    }
    else //发送失败
    {
      f_lose = 1;
      f_last_send_ok = 0;
      return false;
    }
  }
  else
  {
    
    f_lose = 1;
    f_last_send_ok = 0;
    return false;
  }
}

/*****************************************************************************************
发送数据到onnet
******************************************************************************************/
void send_Msg_var_GSM_while_OLED_on(bool a)
{
  // int count=0;
  vTaskDelay(1000L);
  //Serial.printf("time jiange=%lu-%lu=%lu\n",now_unixtime, last_rec_stamp,(now_unixtime - last_rec_stamp));
  last_rec_stamp = now_unixtime;
  // Serial.printf("last_rec_stamp:%d\n",last_rec_stamp);
  if (dbug)
    Serial.println("GSM transmission will start at:" + (String)(period - (now_unixtime - last_rec_stamp)));
  if (workingState == WORKING) //工作模式
  {
    if (f_Flight_Mode == true) //飞行模式
    {
      getTempAndHumilocation(); //采集温湿度，时间戳，没有经纬度(保持上次数据)。
      // saveDataToCSV("/list.csv");
      // postMsgId++; //记录到文件系统
      saveDataToCSV("/lose.csv");
      lose_count++;  //记录到漏发文件
      f_lose = true; //置位标志位
      if (dbug)
      {
        Serial.println("/*****************lose**********************/\n ");
        getAllDataFromCSV("/lose.csv");
        Serial.printf("lose_count = %d\n ", lose_count);
      }
    }
    else //正常发送或有漏传
    {
      client.disconnect(); //客户端断开连接测试
      screen_loopEnabled = false;
      key_attach_null();
      vTaskDelay(200);
      //1.需要联网测网络
      if (a)
      {
        display.clear();
        display.setFont(Roboto_Condensed_12);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 5, "Initializing modem...");
        display.drawProgressBar(5, 50, 118, 8, 5);
        display.display();
      }
      setupModem();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "Waiting for network...");
        display.drawProgressBar(5, 50, 118, 8, 40);
        display.display();
      }
      modemToGPRS();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "getting LBS...");
        display.drawProgressBar(5, 50, 118, 8, 70);
        display.display();
      }
      getLBSLocation();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "connecting to OneNet");
        display.drawProgressBar(5, 50, 118, 8, 90);
        display.display();
      }
      if (!onenet_connect()) //实验过连不上会在连接困难，直接存漏发，下次执行。
      {
        saveDataToCSV("/lose.csv");
        lose_count++; //漏发条数//先将本条加入到LOSE
        f_lose = 1;
        Serial.println("false!\n");
        return;
      }
      else
        Serial.println("OK!");
      getTempAndHumilocation(); //采集温湿度，时间戳，没有经纬度(保持上次数据)。
      // saveDataToCSV("/list.csv");
      // postMsgId++; //记录到文件系统
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "uploading...");
      }

      if (f_lose) //有漏发
      {
        saveDataToCSV("/lose.csv");
        lose_count++;        //漏发条数//先将本条加入到LOSE
        if (lose_count != 0) //连续补漏发
        {
          while (lose_count != f_send_ok)
          {
            jiexi_lose();
            vTaskDelay(500);
          }
        }
        //补发完毕,请标志
        if ((f_send_ok) == lose_count)
        {
          lose_first_flag = true;
          lose_count = 0;
          f_send_ok = 0;
          f_lose = false;
          deleteFile(SPIFFS, "/lose.csv");

          if (old_workingstate == WORKING)
          {
            workingState = WORKING;
            old_workingstate = NOT_WORKING;
          }
        }
      }
      else //正常发送
      {
        set_topcial();
      }
    }
    if (a)
    {
      display.drawProgressBar(5, 50, 118, 8, 100);
      display.display();
      vTaskDelay(200);
      display.setTextAlignment(TEXT_ALIGN_LEFT);
    }
    key_init();
    screen_loopEnabled = true;
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
  }
  else if ((workingState == NOT_WORKING) && (f_lose == true) && (f_Flight_Mode == false)) //自动启动发送，补发漏传
  {
    Serial.println("bufa louchuan");
    old_workingstate = NOT_WORKING;
    workingState = WORKING;
  }
 
}
