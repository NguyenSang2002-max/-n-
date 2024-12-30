#ifndef _MY_CONFIG_H_
#define _MY_CONFIG_H_

const char *ssidAP = "ESP32_IOT";    // Tên wifi AP Mode
const char *passwordAP = "12345678"; // Mật khẩu AP Modev

String Essid = "";  // EEPROM tên wifi nhà bạn
String Epass = "";  // EEPROM mật khẩu wifi nhà bạn
String Etoken = ""; // EEPROM mã token blynk

int EtimeOpenDoor = 0;     // thời gian mở khóa
int EnumberEnterWrong = 0; // số lần nhập sai tối đa
int EtimeLock = 0;         // thời gian khóa khi nhập sai N lần
int EenableChangePass = 0;
int EpassDoor = 1111;

int EtempThreshold1 = 0; // ngưỡng nhiệt độ 1
int EtempThreshold2 = 0; // ngưỡng nhiệt độ 2
int EhumiThreshold1 = 0; // ngưỡng độ ẩm 1
int EhumiThreshold2 = 0; // ngưỡng độ ẩm 2

String snTemp[4] = {"", "Nhiệt độ thấp ", "Nhiệt độ phù hợp ", "Nhiệt độ cao "};
String snHumi[4] = {"", "Độ ẩm thấp ", "Độ ẩm phù hợp ", "Độ ẩm cao "};

#endif