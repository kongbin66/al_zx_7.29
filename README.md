# anleng_zx_demo
2021-7-21 08:57:57
1.安冷云实现了报警发送功能，配合改动了上传tempUA,tempLA上传。
2.将void send_Msg_var_GSM_while_OLED_on(bool a)函数进行了重写，修改了先订阅后上报的顺序。

需要调整：
1.电源电量显示
2.模块低功耗。
2021-7-29 20:14:37
实现电量显示，充电动画。
修改多次获取位置信息，冗余了GSM1/2两种方式。增加了TINYGSM中的LAC,CID获取方式。
