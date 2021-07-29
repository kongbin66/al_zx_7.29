#if !defined(CONFIG_H)
#define CONFIG_H
uint8_t dbug =0;//调试开关



#define TINY_GSM_MODEM_SIM800 //引入TinyGSM库. 在引入之前要定义好TINY_GSM_MODEM_SIM800,让它知道我们用的模块型号
#define GSM1
/***************************头文件******************************************/
#include "Arduino.h"
#include "IP5306.h"
#include "WiFi.h"
#include <esp_sleep.h>
#include "SPIFFS.h"
#include "Wire.h"
#include "uFire_SHT20.h"
#include <TinyGsmClient.h>
#include "PubSubClient.h"

#include "EEPROM.h"
#include "SH1106Wire.h"
#include "images.h"
#include "OneButton.h"
#include <Ds1302.h>
#include "xieyi.h"
#include "ArduinoJson.h"





/*-------------------------------其他硬件定义-------------------------------------*/
#define SerialMon Serial      //调试串口为UART0
#define SerialAT  Serial1      //AT串口为UART1
#define LED 33//LED管脚
/*-------------------------------SIM800相关network.ino---------------------*/
//#define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif
// 创建一个关联到SerialAT的SIM800L模型

// 创建一个GSM型的网络客户端
TinyGsmClient gsmclient(modem);
PubSubClient client(gsmclient);

void setupModem();
void modemToGPRS();
bool getLBSLocation();
DynamicJsonDocument doc(512);


TaskHandle_t task1; //第二核创建一个任务句柄
TaskHandle_t ds_task;
TaskHandle_t xieyi_task;

time_t unixtime(void) ;
time_t sys_sec=0;

#define PIN_I2C_SDA 21 
#define PIN_I2C_SCL 22
SH1106Wire display(0x3c, 21, 22);


/********************电量采集相关*************************************/
IP5306 ip5306;
//#define BATTERY_ADC_PIN  36    //电量ADC采集管脚后续改到ADC1上，避免影响WIFI

#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00
#define Power_min_voltage 3.3//设定最小关机电压

#define grade30 3.4
#define grade50 3.5
#define grade80 3.7
#define grade100 4.2

uint8_t POWER_warning_flag;//电压报警标志 0：正常 1：欠压
//电池充电管理
bool Battery_test();
//周期检测电量程序
void dianliangloop();








/*-------------------------------出厂设置定义-------------------------------------*/

#define FACTORY_SLEEPTIME   30     // 300000000    //休眠时间        只适用一次
#define FACTORY_TEMP_LIMIT_ENABLE 0    //出厂温度上下限失能
#define FACTORY_TEMP_UPPER_LIMIT 50.0  //出厂温度上限
#define FACTORY_TEMP_LOWER_LIMIT -40.0 //出厂温度下限
#define FACTORY_DATE_YEAR 1970         //出厂默认时间
#define FACTORY_DATE_MONTH 1           //出厂默认时间
#define FACTORY_DATE_DAY 1             //出厂默认时间
#define FACTORY_TIME_HOUR 0            //出厂默认时间
#define FACTORY_TIME_MIN 0             //出厂默认时间





/*-------------------------------SIM800L 硬件定义----------------------------------*/

//#define MODEM_RST 5      //SIM800L复位引脚接在GPIO5
//#define MODEM_PWRKEY  //SIM800L开关机引脚接在GPIO32
#define MODEM_POWER_ON 32 //SIM800L电源引脚接在GPIO4
#define MODEM_TX 27       //SIM800L串口TX引脚接在GPIO27
#define MODEM_RX 26       //SIM800L串口RX引脚接在GPIO26

/*-------------------------------系统时间定义-------------------------------------*/
RTC_DATA_ATTR uint32_t now_unixtime;//现在系统时间 *
RTC_DATA_ATTR unsigned long start_time;//起始记录时间
RTC_DATA_ATTR unsigned long last_time;//最后记录时间

time_t time_last_async_stamp;//上一次的时间戳




/**********************************ds1302相关**************************************/
//ds1302驱动引脚
#define PIN_ENA 5
#define PIN_CLK 19
#define PIN_DAT 18
//创建DS1302对象
Ds1302 ds_rtc(PIN_ENA, PIN_CLK, PIN_DAT);
Ds1302::DateTime now1;//ds1302读取的时间










/*********************************显示屏相关函数 al_oled.ino***************************/
//显示/按键相关定义/
//OLED状态
#define OLED_ON 1
#define OLED_OFF 0

//工作状态
#define NOT_WORKING 0//停止工作
#define WORKING 1//工作
//显示状态
#define MAIN_TEMP_SCREEN 0 //温度界面
#define MAIN_HUMI_SCREEN 1 //湿度界面
#define TIPS_SCREEN 2  //单击后提示
#define LOGO_SCREEN 4 //开机
#define BLE_SCREEN 5  //蓝牙
#define TEMP_HUMI_SCROLL_SCREEN 6 //滚屏温度
#define HUMI_TEMP_SCROLL_SCREEN 7 //滚屏湿度
#define SETTING_SUCCESS 8 //设定成功
#define REC_START_SCREEN 9 //开始记录
#define REC_STOP_SCREEN 10//停止记录
#define REC_COUNT_SCREEN 11 //计数
#define fxmod_ON 12 //飞行模式开
#define fxmod_OFF 13 //飞行模式关

void showWelcome();
void screen_loop();
void screen_show();
void showsim800init_Screen();
//OLED状态标志切换
void oled_on_off_switch(); 
void showBFLC_Screen();
const char *p1=F16x16_b100;//电量图标指针
//充电动画
void Battery_Charge();

//按键状态
#define NOKEYDOWN 0
#define CLICK 1
#define DOUBLECLICK 2
#define LONGPRESS_START 3
#define LONGPRESS_END 4
#define LONGPRESS_DURRING 5
//state of rec_State
#define START_RECING 0
#define END_RECING 1
#define KEEP_RECING 2




RTC_DATA_ATTR int workingState;        //工作状态机

RTC_DATA_ATTR int oledState;           //OLED工作状态机
RTC_DATA_ATTR int screenState;         //屏幕状态机
RTC_DATA_ATTR bool screen_loopEnabled; //是否允许滚屏
RTC_DATA_ATTR int current_rec_State;   //当前记录状态机 (正在开始记录,正在持续记录,正在停止记录)
time_t loopStartTime;                  //计算主屏幕滚屏的起始时间
time_t loopnowTime;                    //计算主屏幕滚屏的当前时间
time_t looptimeSpan;                   //计算滚屏间隔时间

time_t screen_On_Start;                //计算息屏的亮屏起点
time_t screen_On_now;                  //计算息屏的当前时间
time_t screen_On_last_span;            //亮屏时间间隔
time_t screen_Off_to_sleep_span;       //息屏到休眠时间间隔
time_t show_tip_screen_last;           //提示界面自动返回的时间
time_t show_BLE_screen_last;           //蓝牙界面自动返回的时间
time_t show_rec_stop_screen_last;      //停止测量界面自动返回的时间
time_t last_rec_stamp;                 //上次休眠时间


time_t now_number = 0;
time_t start_number = 0;
int now_state;
 void showStatusBar();




 //电池充电管理
bool Battery_test();
//周期检测电量程序
void dianliangloop();

/*******************************设备码***********************************/
#if 1
const char *mqtt_server = "218.201.45.7"; //onenet 的 IP地址
const int port = 1883;                     //端口号
#define mqtt_devid "al_kh00001_zx_0001"         //设备ID
#define mqtt_pubid "4LwKzUwOpX"                //产品ID
//鉴权信息
#define mqtt_password "version=2018-10-31&res=products%2F4LwKzUwOpX%2Fdevices%2Fal_kh00001_zx_0001&et=4092599349&method=md5&sign=xpaXrOTMJ9WJjOVolwJhWw%3D%3D"
#endif

#if 0
const char *mqtt_server = "218.201.45.7"; //onenet 的 IP地址
const int port = 1883;                     //端口号
#define mqtt_devid "al_kh00001_zx_0002"         //设备ID
#define mqtt_pubid "4LwKzUwOpX"                //产品ID
//鉴权信息
#define mqtt_password "version=2018-10-31&res=products%2F4LwKzUwOpX%2Fdevices%2Fal_kh00001_zx_0002&et=4092599349&method=md5&sign=FxSayE%2BpBzK9L1YgXt8rxA%3D%3D"
#endif

#if 0
const char *mqtt_server ="218.201.45.7";// "183.230.102.116"; //onenet 的 IP地址
const int port = 1883;                     //端口号
#define mqtt_devid "al_kh00001_zx_0003"         //设备ID
#define mqtt_pubid "4LwKzUwOpX"                //产品ID
//鉴权信息
#define mqtt_password "version=2018-10-31&res=products%2F4LwKzUwOpX%2Fdevices%2Fal_kh00001_zx_0003&et=4092599349&method=md5&sign=RJjI9dBTNLUXL9rk9zbBtQ%3D%3D"
#endif
/*-------------------------------云平台相关定义-------------------------------------*/
//设备上传数据的post主题
#define ONENET_TOPIC_PROP_POST "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/post"
//接收下发属性设置主题
#define ONENET_TOPIC_PROP_SET "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set"
//接收下发属性设置成功的回复主题
#define ONENET_TOPIC_PROP_SET_REPLY "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set_reply"

//接收设备属性获取命令主题
#define ONENET_TOPIC_PROP_GET "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get"
//接收设备属性获取命令成功的回复主题
#define ONENET_TOPIC_PROP_GET_REPLY "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get_reply"
//设备期望值获取请求主题
#define ONENET_TOPIC_DESIRED_GET "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/desired/get"
//设备期望值获取响应主题
#define ONENET_TOPIC_DESIRED_GET_RE "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/desired/get/reply"

//设备期望值删除请求主题
#define ONENET_TOPIC_DESIRED_DEL "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/desired/delete"
//设备期望值删除响应主题
#define ONENET_TOPIC_DESIRED_DEL_RE "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/desired/delete/reply"
//这是post上传数据使用的模板
#define ONENET_POST_BODY_FORMAT "{\"id\":\"1\",\"params\":%s}"

#define mrtt_topical "$sys/%s/%s/cmd/request/#"//订阅主题
//#define mrtt_topical      "$sys/%s/%s/thing/property/post"
#define mrtt_topical_reply "$sys/%s/%s/thing/property/post/reply"
/*-------------------------------公共变量,参数定义-------------------------------------*/
//温湿度采集相关
float currentTemp;
float currentHumi;
//F_温湿度读取标志
bool tempAndHumi_Ready;
bool timeNTPdone;
//判断是否第一次启动


#define BLE_ON 1
#define BLE_OFF 2
#define LOCKED 1
#define UNLOCKED 2
#define QUALITIFY_RIGHT 1
#define QUALITIFY_WRONG 2

RTC_DATA_ATTR int bleState;       //蓝牙状态机
RTC_DATA_ATTR int lockState;      //开关箱状态机
RTC_DATA_ATTR int qualifiedState; //合格状态机

/*-------------------------------公共变量,参数定义-------------------------------------*/
//以下参数需要休眠RTC记忆
RTC_DATA_ATTR bool  temp_alarm=0;      // tempLimit_enable;    //温度上下限报警开关
RTC_DATA_ATTR float tempU=50;//tempUpperLimit;                  //温度上限设定
RTC_DATA_ATTR float tempL=-100;//tempLowerLimit;                  //温度下限设定
RTC_DATA_ATTR long tempUA=0;                        //温度上限报警计数
RTC_DATA_ATTR long tempLA=0;                        //温度下限报警计数
RTC_DATA_ATTR time_t period;                      //休眠时间
RTC_DATA_ATTR time_t sleep_start_time;               //休眠开始时间
RTC_DATA_ATTR time_t sleep_end_time;                 //休眠结束时间
RTC_DATA_ATTR time_t sleep_time_count;               //休眠时长时间
RTC_DATA_ATTR int postMsgId = 0;                     //记录已经post了多少条
RTC_DATA_ATTR float locationE, locationN, locationA; //地理位置,经度纬度
RTC_DATA_ATTR float locationE_, locationN_, locationA_; //地理位置,经度纬度

RTC_DATA_ATTR int timeNow_Y, timeNow_M, timeNow_D, timeNow_h, timeNow_m, timeNow_s;


//基站定位参数：
int32_t   LAC,CIID; //位置大区号，小区号
// cid;//※基站码
// lac;//※地区区域码
// mcc;//※移动网号
// mnc;//※移动国家号码
// networktype="LTE";//网络制式
// ss;//信号强度
// ta;//移动台距已确定其发往基站的定时超前量
// flag=10;//基站信息数值进制
//  {
//   "id": "1",
//   "version": "1.0",
//   "params": {
//     "$OneNET_LBS": {
//         "value":[{
//             "cid":21089,
//             "lac":6334,
//             "mcc":460,
//             "mnc":0,
//             "networkType":5,
//             "ss":-12,
//             "ta":1,
//             "flag":10
//             }
//         ]
//     }
//   }
// }

/*-------------------------------SPIFFS定义-------------------------------------*/
RTC_DATA_ATTR bool alFFS_thisRec_firstData_flag; //本次记录第一次上传
RTC_DATA_ATTR char nowREC_filepath[21];          //记录文件的路径
uint32_t lose_count = 0;//漏发文件现在信息条数
bool lose_first_flag=0;//漏发文件第一次标志
char work_data[512];//漏发暂存数组
int work_data_num =0;//漏发工作表数据指针
//读文件列表
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);


//删除文件
void deleteFile(fs::FS &fs, const char * path);





/*-------------------------------初始化相关init.ino-------------------------------------*/
void hardware_init();
void software_init();

/*-------------------------------温湿度采集相关al_sht20.ino-----------------------------*/
uFire_SHT20 sht20;



// /*-------------------------------ali_mqtt服务相关ali_mqtt.ino---------------------*/
// void ali_mqtt_connect();
// void ali_callback(char *topic, byte *payload, unsigned int length);
// void ali_sendTemp_Humi_LBS();
/*-------------------------------onenet_mqtts服务相关onenet_mqtts.ino---------------------*/
bool f_MQTT_ON=0;
bool sendTempAndHumi();//发送数据




bool onenet_connect();
/***************获得期望值********************
1.订阅期望值请求和响应主题，设置回应主题回调函数。
2.发送temp_alarm/tempL/tempU/period到期望值
*********************************************/
void getDesired();

/*-------------------------------休眠服务相关al_sleep.ino---------------------*/
void go_sleep_a_while_with_ext0();//进入休眠

/*--------------------------------eeprom相关函数--------------------*/

void get_eeprom_firstBootFlag();
void eeprom_config_init();
void eeprom_config_save_parameter(void);
void eeprom_config_set(bool temp_alarm, uint32_t time1, float tempU, float tempL);
/*********************************SPIFFS相关函数 al_FFS.ino**********/
void alFFS_init();
void alFFS_addRec();








/*********************************按键相关函数 al_key1.ino***********/

#define KEY1      14            //按键1对应引脚
#define WEAKUPKEY1 GPIO_NUM_14 //按键1对应引脚
OneButton button(KEY1, true);
#define KEY2      13
#define WEAKUPKEY2 GPIO_NUM_13 //按键1对应引脚
OneButton button2(KEY2, true);
RTC_DATA_ATTR int keyState;            //按键状态机
time_t keyScreen_Start;                //计算按键触发的当前屏的起始时间
time_t keyScreen_Now;                  //计算当前屏的当前时间
void key_init();
void key_loop();
void key_attach_null();
void oledoff_upload_but_click();

/*********************************对时相关函数 al_time.ino***********/
void wakeup_init_time();
void waking_update_time();

/*********************时间相关**************************************************/

void SET_SLEEPTIME(time_t t);

bool firstBootFlag; //第一次启动标志位
bool list_first_flag=0;//记录文件第一次标志

bool f_Flight_Mode=0;//飞行模式
bool f_lose=0;  //有漏发标志
bool old_workingstate = 0;


uint32_t f_send_ok=1;//漏发上传成功条数

/*******************************************sht20.ino*************************************************/
/***********获取温湿度************
 当温度在一个合理范围内再更新
 温湿度寄存器。
 条件：(_currentTemp < 100)&&(_currentTemp > -40)满足
 更新：currentTemp，currentHumi
      tempAndHumi_Ready
 ********************************/
void sht20getTempAndHumi();


/*******************************************al_csv.ino************************************************/
//上传参数
unsigned long tmsp ;//时间戳
float tp ;//温度
float h ;//湿度
float e ;//经度
float n ;//纬度

/****采集数据打印到savedata****
 采集：温度湿度/经纬度/系统时间戳
 输出：格式打印到savedata[48];
******************************/
void getTempAndHumilocation();
/****将savedata记录到文件系统****/
void saveDataToCSV(String file_add);
/***********读取CSV全部数据***********/
void getAllDataFromCSV(String file_add);
void getADataFromCSV(String file_add);//在文件系统中读取并删除第一条
void parseData(long* _tmsp,float* _tp,float* _h,float* _e,float* _n);//解析CSV
#endif // CONFIG_H