#include "config.h"
#include "TinyGsmCommon.h"
#include "TinyGsmClientSIM800.h"
#include "TinyGsmModem.tpp"

/*-------------------------------APN相关定义-------------------------------------*/
const char apn[] = "CMNET"; // Your APN
const char gprsUser[] = ""; // User
const char gprsPass[] = ""; // Password
const char simPIN[] = "";   // SIM card PIN code, if any

/*-------------------------------初始化SIM800L-------------------------------------*/
void setupModem()
{

  // pinMode(MODEM_PWRKEY, OUTPUT);   //开关机键引脚
  // 先打开SIM800L的电源
  digitalWrite(MODEM_POWER_ON, HIGH);
  //根据手册要求拉下PWRKEY 1秒钟以上 可以开机
  // digitalWrite(MODEM_PWRKEY, HIGH);
  //  digitalWrite(MODEM_PWRKEY, LOW);
  vTaskDelay(1000L);
  // digitalWrite(MODEM_PWRKEY, HIGH);
  SerialMon.println("Initializing modem...");
  modem.restart();
  modem.init(); //开机后modem初始化一下
}
/*-------------------------------SIM800L连接GPRS-------------------------------------*/
void modemToGPRS()
{
  //连接网络
  SerialMon.print("Waiting for network...");
  while (!modem.waitForNetwork(240000L))
  {
    SerialMon.print(".");
    vTaskDelay(500);
  }
  SerialMon.println(" OK");
  //连接GPRS接入点
  SerialMon.print(F("Connecting to APN: "));
  SerialMon.print(apn);
  while (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    SerialMon.print(".");
    vTaskDelay(500);
  }
  SerialMon.println(" OK");
  SerialMon.print("signalQuality:");
  Serial.println(modem.getSignalQuality());
}
/*-------------------------------获取位置信息-------------------------------------*/
#ifdef GSM1
//如果获取位置和时间异常返回0
bool getLBSLocation()
{
  int i=0;
  float lat = 0;
  float lon = 0;
  float accuracy = 0;
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;

  Serial.println("getting LBS...");
  for (i = 0; i < 5; i++)
  {
    if (modem.getGsmLocation(&lat, &lon, &accuracy, &year, &month, &day, &hour, &min, &sec))
    {
      Serial.printf("%d-%d-%d %d:%d:%d\n", year, month, day,hour, min, sec);
      Serial.printf("Latitude:%f,Longitude:%f,Accuracy:%f\n", lat, lon, accuracy);

      if (lat > 1) //位置信息更新
      {
        locationE_ = lat;
        locationN_ = lon;
        locationA_ = accuracy;
      }
      if (year != 0 && month != 0 && day != 0) //数据读出错误
      {
        timeNow_Y = year;
        timeNow_M = month;
        timeNow_D = day;
        timeNow_h = hour;
        timeNow_m = min;
        timeNow_s = sec;

        now1.year = timeNow_Y - 2000;
        now1.month = timeNow_M;
        now1.day = timeNow_D;
        now1.hour = timeNow_h;
        now1.minute = timeNow_m;
        now1.second = timeNow_s;
        ds_rtc.setDateTime(&now1);
        now_unixtime = unixtime();
        time_last_async_stamp = now_unixtime;
      }
      break;
    }
    Serial.printf("Reconnection:%d\n",i);
  }
  if (i< 5) return true;
  else return false;
}



#endif
#ifdef GSM2
bool getLBSLocation()
{
  String lac;
  String ciid;
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;
  float timezone = 0;
  int count = 0;
  int splitsybol[3];
  bool res = 0;
  char buf[4];
  char *p2;
  char *str;

  Serial.println("getting LBS...");
  String Position = modem.getGsmLocation1();
  res = modem.getNetworkTime(&year, &month, &day, &hour, &min, &sec, &timezone);
#if 0
    Serial.println(year);
    Serial.println(month);
    Serial.println(day);
    Serial.println(hour);
    Serial.println(min);
    Serial.println(sec);
    Serial.println(timezone);
#endif

  if ((Position.length() >= 17) && (res == 1))
  {
    //1.处理大小区号
    for (size_t i = 0; i < Position.length(); i++)
    {
      if (Position[i] == ',')
      {
        splitsybol[count] = i;
        count++;
      }
    }
    lac = Position.substring(splitsybol[1] + 1, splitsybol[2]).c_str(); //c_str()函数返回一个指向正规C字符串的指针常量, 内容与本string串相同.
    ciid = Position.substring(splitsybol[2] + 1, splitsybol[3]).c_str();
    // Serial.println(LAC);
    // Serial.println(CIID);
    lac.replace("\"", "");
    ciid.replace("\"", "");
    //  Serial.println(LAC);
    //  Serial.println(CIID);
    lac.toCharArray(buf, 5, 0);
    //Serial.println(buf);
    p2 = buf;
    LAC = strtol(p2, &str, 16);
    Serial.println(LAC);
    ciid.toCharArray(buf, 5, 0);
    //Serial.println(buf);
    p2 = buf;
    CIID = strtol(p2, &str, 16);
    Serial.println(CIID);
    //2.存储数据

    timeNow_Y = year;
    timeNow_M = month;
    timeNow_D = day;
    timeNow_h = hour;
    timeNow_m = min;
    timeNow_s = sec;
    now1.year = timeNow_Y - 2000;
    now1.month = timeNow_M;
    now1.day = timeNow_D;
    now1.hour = timeNow_h;
    now1.minute = timeNow_m;
    now1.second = timeNow_s;
    ds_rtc.setDateTime(&now1);
    now_unixtime = unixtime();
    time_last_async_stamp = now_unixtime;
  }
}
#endif
/*\u0035\u0033\u0038\u0045\u000a
538E
10: 53 51 56 69
16: 35 33 38 45
*/