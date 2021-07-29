#include "config.h"

void nullfunc()
{
  
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
}
void doubleclick()
{
  if (oledState == OLED_ON)
  {
    Serial.println("doubleclick");
    keyState = DOUBLECLICK;
  }
  screen_On_Start = sys_sec;
  screen_On_now = sys_sec;
}
void click()
{
  if (oledState == OLED_ON)
  {
    Serial.println("click");
    keyState = CLICK;
  }
  screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
}
void longPressStart()
{
  if (oledState == OLED_ON)
  {
    Serial.println("longPressStart");
    keyState = LONGPRESS_START;
  }
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
}
void duringLongPress()
{
  if (oledState == OLED_ON)
  {
   
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
  }
  if (button.isLongPressed())
  {
    Serial.print("duringLongPress:");
    Serial.println(button.getPressedTicks());
    keyScreen_Start = sys_sec; //处理长按界面下的长按
    keyScreen_Now = sys_sec;   //处理长按界面下的长按
    delay(50);
    keyState = LONGPRESS_DURRING;
  }
}
void longPressStop()
{
  // screen_On_Start = millis();
  // screen_On_now = millis();
  screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
  Serial.println("longPressStop");
  keyState = LONGPRESS_END;
}
//button2长按关机
//由于唤醒功能同在此管脚，采用息屏后暗示放手，5秒后设备才关机，如果5秒内不放手的话又会开机
void duringLongPres2()
{
  Serial.printf("into \n");
  Serial.println(button2.getPressedTicks());
 if(oledState == OLED_ON)
 {
     workingState = NOT_WORKING;
    digitalWrite(LED,LOW); 
    list_first_flag=true;
    lose_first_flag=true;
    postMsgId = 0; //清记录条数
    lose_count=0;

     
     display.displayOff(); 
     digitalWrite(LED,HIGH);
     delay(5000);
     esp_sleep_enable_ext0_wakeup(WEAKUPKEY2, LOW); //使能按键唤醒
     digitalWrite(MODEM_POWER_ON, LOW);
     gpio_hold_en(GPIO_NUM_32);                     //锁定电源管脚
     gpio_deep_sleep_hold_en();
     esp_deep_sleep_start(); 
 }
}
void key_loop()
{
  switch (keyState)
  {
  case CLICK:
    if (screenState == MAIN_TEMP_SCREEN || screenState == MAIN_HUMI_SCREEN)
    {
      screen_loopEnabled = false;
      screenState = TIPS_SCREEN;
      keyScreen_Start = sys_sec;
      keyScreen_Now = sys_sec;
      keyState = NOKEYDOWN;
    }
    else if (screenState == TIPS_SCREEN || screenState == BLE_SCREEN || screenState == REC_STOP_SCREEN|| screenState == REC_START_SCREEN)
    {
      screen_loopEnabled = true;
      Serial.println("anjianfanhui");
      keyState = NOKEYDOWN;
    }
    break;
  case DOUBLECLICK:
    if (screenState == MAIN_TEMP_SCREEN || screenState == MAIN_HUMI_SCREEN || screenState == TIPS_SCREEN)
    {
      if(f_Flight_Mode==true)//正在飞行模式，关闭。
      {
         f_Flight_Mode=false;
         workingState=NOT_WORKING;
         screen_loopEnabled = false;
         screenState = fxmod_OFF;
         qualifiedState = QUALITIFY_WRONG ;
         Serial.printf("flight off\n");
         keyScreen_Start = sys_sec;
         keyScreen_Now = sys_sec;
         keyState = NOKEYDOWN;

      }
      else if(f_Flight_Mode==false)//关闭时飞行模式，打开
      {
         f_Flight_Mode=true;
         workingState=WORKING;
         screen_loopEnabled = false;
         screenState = fxmod_ON;
         
         qualifiedState = QUALITIFY_RIGHT;
         Serial.printf("flight on\n");
         keyScreen_Start = sys_sec;
         keyScreen_Now = sys_sec;
         keyState = NOKEYDOWN;
      }
    }
    break;
  case LONGPRESS_END:
    if (screenState == MAIN_TEMP_SCREEN || screenState == MAIN_HUMI_SCREEN || screenState == TIPS_SCREEN)
    {
      if (workingState == NOT_WORKING)
      {
        screen_loopEnabled = false;
        screenState = REC_START_SCREEN;
        workingState = WORKING;
        current_rec_State = START_RECING;
        alFFS_thisRec_firstData_flag = true;
        Serial.println("workingState = WORKING;");
        keyState = NOKEYDOWN;
        tempUA=tempLA=0;
        last_rec_stamp=0;
      }
      else if (workingState == WORKING)
      {
        screen_loopEnabled = false;
        screenState = REC_STOP_SCREEN;
        workingState = NOT_WORKING;
        current_rec_State = END_RECING;
        //alFFS_endRec();
        Serial.println("workingState = NOT_WORKING;1");
        keyScreen_Start = sys_sec;
        keyScreen_Now = sys_sec;
        keyState = NOKEYDOWN;
        tempUA=tempLA=0;
      }
    }
    keyState = NOKEYDOWN; //处理长按界面下的长按
    break;
  default:
    break;
  }
}
void key_init()
{
  button2.reset(); //清除一下按钮状态机的状态
  button2.setPressTicks(2000);//设置长按毫秒数
  button2.attachDuringLongPress(duringLongPres2);
  button.reset(); //清除一下按钮状态机的状态
  button.attachClick(click);
  button.attachDoubleClick(doubleclick);
  button.attachLongPressStart(longPressStart);
  button.attachDuringLongPress(duringLongPress);
  button.attachLongPressStop(longPressStop);

}
void key_attach_null()//
{
  button.attachClick(nullfunc);
  button.attachDoubleClick(nullfunc);
  button.attachLongPressStart(nullfunc);
  button.attachDuringLongPress(nullfunc);
  button.attachLongPressStop(nullfunc);
  // button2.attachClick(nullfunc);
  // button2.attachDoubleClick(nullfunc);
  // button2.attachLongPressStart(nullfunc);
  // button2.attachDuringLongPress(nullfunc);
  // button2.attachLongPressStop(nullfunc);
}

void oledoff_upload_but_click()
{
  Serial.println("oledoff_upload_but_click");
  display.displayOn();
  display.drawString(0, 28, "wait");
  delay(200);
  display.displayOff();
}