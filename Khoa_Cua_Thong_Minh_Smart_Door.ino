//-------------------------------------Nguyễn Hữu Sáng -----------------------------------//

#define BLYNK_PRINT Serial
// You should get Auth Token in the Blynk App.
#define BLYNK_TEMPLATE_ID "TMPL6MYlKj_iY"
#define BLYNK_TEMPLATE_NAME "SMART DOOR"
char BLYNK_AUTH_TOKEN[32] = "";
// Import required libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SimpleKalmanFilter.h>
#include "index_html.h"
#include "data_config.h"
#include <EEPROM.h>
#include <Arduino_JSON.h>
#include "icon.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//-----------------------GPIOConfig --------------------------------------

#define OLED_SDA 21 // ok
#define OLED_SCL 22 // ok

// Khai bao LED
#define LED 33 // ok

// Khai báo RELAY
#define RELAY 25 // ok

// Khai báo BUZZER
#define BUZZER 2 // ok

// Khai báo DHT11
#define DHT11_PIN 26 // ok

// Khai báo button
#define BUTTON_DOWN_PIN 34  // ok
#define BUTTON_UP_PIN 35    // ok
#define BUTTON_SET_PIN 32   // ok
#define BUTTON_OPEN_DOOR 12 // ok

// Định nghĩa các chân của bàn phím kết nối với MCP23017
#define KEYPAD_ROW1 4 // GPA0
#define KEYPAD_ROW2 7 // GPA1
#define KEYPAD_ROW3 6 // GPA2
#define KEYPAD_ROW4 0 // GPA3
#define KEYPAD_COL1 1 // GPA4
#define KEYPAD_COL2 2 // GPA5
#define KEYPAD_COL3 3 // GPA6

// Khai bao MFRC522 RFID
#define SS_PIN 13    // ok
#define RST_PIN 15   // ok
#define RFID_SCK 18  // ok
#define RFID_MISO 19 // ok
#define RFID_MOSI 23 // ok

// Định nghĩa chân RX và TX cho cảm biến AS608
#define AS608_RX 17
#define AS608_TX 16

//----------------------- Khai báo 1 số biến Blynk -----------------------
bool blynkConnect = true;
BlynkTimer timer;
// Một số Macro
#define ENABLE 1
#define DISABLE 0
// ---------------------- Khai báo cho OLED 1.3 --------------------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3C // initialize with the I2C addr 0x3C Typically eBay OLED's
// #define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    //   QT-PY / XIAO
Adafruit_SH1106G oled = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define SAD 0
#define NORMAL 1
#define HAPPY 2

typedef enum
{
  SCREEN0,
  SCREEN1,
  SCREEN2,
  SCREEN3,
  SCREEN4,
  SCREEN5,
  SCREEN6,
  SCREEN7,
  SCREEN8,
  SCREEN9,
  SCREEN10,
  SCREEN11,
  SCREEN12,
  SCREEN13
} SCREEN;
int screenOLED = SCREEN0;

bool enableShow1 = DISABLE;
bool enableShow2 = DISABLE;
bool enableShow3 = DISABLE;
bool autoWarning = DISABLE;

// Khai bao LED
#define LED_ON 0
#define LED_OFF 1

uint32_t timeCountBuzzerWarning = 0;
int warningTempState = SAD;
int warningHumiState = NORMAL;
int warningDustState = HAPPY;

String OLED_STRING1 = "Xin chao";
String OLED_STRING2 = "Hi my friend";
String OLED_STRING3 = "Canh bao";
// ----------------------------- Khai báo Keypad -------------------------------------------
#include <Adafruit_MCP23X17.h>
#include <Keypad_MC17.h>
#include <QueueArray.h> // Thư viện hỗ trợ hàng đợi

#define I2CADDR 0x20 // Địa chỉ I2C của MCP23017 (có thể thay đổi nếu cần)
#define KEY_JUST_PRESSED 0
#define KEY_PRESSED 1
#define KEY_JUST_RELEASED 2
#define KEY_RELEASED 3

// Khai báo MCP23017
Adafruit_MCP23X17 mcp;
const uint16_t queueSize = 10;

const byte ROWS = 4; // rows
const byte COLS = 3; // columns

// define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte rowPins[ROWS] = {KEYPAD_ROW1, KEYPAD_ROW2, KEYPAD_ROW3, KEYPAD_ROW4};
byte colPins[COLS] = {KEYPAD_COL1, KEYPAD_COL2, KEYPAD_COL3};

Keypad_MC17 customKeypad = Keypad_MC17(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR);

typedef enum
{
  IDLE_MODE,
  WAIT_CLOSE,
  CHANGE_PASSWORD,
  LOCK_1MIN
} MODE;

typedef struct
{
  uint8_t EVENT;
  char KEY;
} keypadEvent;

uint8_t modeRun = IDLE_MODE;
uint8_t allowAccess = 0;
uint8_t countError = 0;
uint8_t countKey = 0;
char passwordEnter[5] = {0};
char password[5] = "1111"; // mật khẩu mặc định
char keyMode;
uint32_t timeMillisAUTO = 0;
uint8_t notiOne = 0;

typedef enum
{
  ENTER_PASS_1,
  ENTER_PASS_2
} CHANGEPASS;

uint8_t changePassword = ENTER_PASS_1;
char passwordChange1[5] = {0}; // mode đổi pass,mảng lưu pass lần 1
char passwordChange2[5] = {0}; // mode đổi pass,mảng lưu pass lần 2
char starkey[8];

QueueArray<keypadEvent> keyEventQueue(queueSize);

void tick()
{
  customKeypad.getKeys(); // Cập nhật trạng thái của bàn phím

  for (int i = 0; i < LIST_MAX; i++)
  {
    if (customKeypad.key[i].stateChanged)
    {
      keypadEvent e;
      e.KEY = customKeypad.key[i].kchar;

      switch (customKeypad.key[i].kstate)
      {
      case PRESSED:
        e.EVENT = KEY_JUST_PRESSED;
        break;
      case HOLD:
        e.EVENT = KEY_PRESSED;
        break;
      case RELEASED:
        e.EVENT = KEY_JUST_RELEASED;
        break;
      case IDLE:
        e.EVENT = KEY_RELEASED;
        break;
      }

      // Thêm sự kiện vào hàng đợi
      if (!keyEventQueue.isFull())
      {
        keyEventQueue.push(e);
      }
    }
  }
}

keypadEvent read()
{
  if (!keyEventQueue.isEmpty())
  {
    return keyEventQueue.pop();
  }
  else
  {
    // Trả về một sự kiện không hợp lệ nếu hàng đợi trống
    keypadEvent emptyEvent;
    emptyEvent.EVENT = 255; // Giá trị không hợp lệ
    emptyEvent.KEY = '\0';
    return emptyEvent;
  }
}

// -----------------------------------------------------------------------

//------------------ Khai báo RFID --------------------------------------

#include <MFRC522.h> // Thư viện giao tiếp với module RFID-RC522
#include <vector>
#include <array>

MFRC522 mfrc522(SS_PIN, RST_PIN); // Khởi tạo đối tượng MFRC522

// Danh sách các UID hợp lệ
std::vector<std::array<byte, 4>> allowedUIDs = {
    {{0x93, 0x9A, 0x09, 0xEC}},
    {{0x12, 0x34, 0x56, 0x78}},
    {{0x12, 0x34, 0x56, 0x78}}
    // Thêm UID khác nếu cần
};

std::vector<String> allowedUIDLevels = {
    "Master",
    "Basic",
    "Basic"
    // Thêm cấp độ tương ứng
};

// std::vector<std::array<byte, 4>> allowedUIDs = {
//     { {0x93, 0x9A, 0x09, 0xEC} },
//     { {0x12, 0x34, 0x56, 0x78} }
//     // Thêm UID khác nếu cần
// };
// const int allowedUIDCount = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

size_t allowedUIDCount = allowedUIDs.size();
bool statusRFID = DISABLE;              // Biến để kiểm tra trạng thái RFID (ENABLE: đã tắt, DISABLE: chưa tắt)
unsigned long lastCardDetectedTime = 0; // Lưu thời gian thẻ cuối cùng được phát hiện
int reactivationCounter = 0;            // Biến đếm số lần quẹt thẻ để kích hoạt lại RFID

//-------------------- Khai báo Button-----------------------
#include "mybutton.h"

#define BUTTON1_ID 1
#define BUTTON2_ID 2
#define BUTTON3_ID 3
#define BUTTON4_ID 4
Button buttonSET;
Button buttonDOWN;
Button buttonUP;
Button buttonOpen;
void button_press_short_callback(uint8_t button_id);
void button_press_long_callback(uint8_t button_id);
//------------------------------------------------------------
TaskHandle_t TaskButton_handle = NULL;
TaskHandle_t TaskOLEDDisplay_handle = NULL;
TaskHandle_t TaskDHT11_handle = NULL;
TaskHandle_t TaskRFID_handle = NULL;
TaskHandle_t TaskAS608_handle = NULL;

// --------------------- Cảm biến DHT11 ---------------------
#include "DHT.h"

#define DHTTYPE DHT11
DHT dht(DHT11_PIN, DHTTYPE);
float tempValue = 30;
int humiValue = 60;
SimpleKalmanFilter tempfilter(2, 2, 0.001);
SimpleKalmanFilter humifilter(2, 2, 0.001);
bool dht11ReadOK = true;

//-----------------------------------------------------------

//--------------------- Cảm biến vân tay AS608-----------------//
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

bool statusAS608 = ENABLE; // Biến để kiểm tra trạng thái AS608 (ENABLE: hoạt động, DISABLE: đã tắt)

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Function prototypes
void TaskAS608(void *pvParameters);
uint8_t getFingerprintID();
void sendSleepCommand();
void sendWakeCommand();

//--------------------------------------------------------------//

void setup()
{
  Serial.begin(115200);

  // Đọc data setup từ eeprom
  EEPROM.begin(512);
  readEEPROM();

  // Khởi tạo RELAY
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, DISABLE);

  // Khởi tạo BUZZER
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, DISABLE);

  // Khởi động MCP23017 và bàn phím
  Wire.begin(); // Khởi tạo I2C
  customKeypad.begin(makeKeymap(keys));
  Wire.beginTransmission(I2CADDR);
  Wire.write(0x0C); // Thanh ghi GPPUA
  Wire.write(0xFF); // Kích hoạt pull-up cho GPA0-GPA7
  Wire.endTransmission();

  // Khởi tạo LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LED_OFF);

  // Khởi tạo OLED
  oled.begin(i2c_Address, true);
  oled.setTextSize(2);
  oled.setTextColor(SH110X_WHITE);

  // Khởi tạo DHT11
  dht.begin();
  SPI.begin(); // Khởi tạo bus SPI

  //  Khởi tạo MFRC522

  // Thiết lập UART cho cảm biến vân tay

  // ---------- Đọc giá trị AutoWarning trong EEPROM ----------------
  autoWarning = EEPROM.read(210);
  //----------- Đọc pass trong eeprom ----------------------------
  savePASStoBUFF();
  EpassDoor = convertPassToNumber();

  // Khởi tạo nút nhấn
  pinMode(BUTTON_OPEN_DOOR, INPUT_PULLUP); // Nút bấm kết nối với GPIO 12
  pinMode(BUTTON_SET_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  button_init(&buttonOpen, BUTTON_OPEN_DOOR, BUTTON4_ID);
  button_init(&buttonSET, BUTTON_SET_PIN, BUTTON1_ID);
  button_init(&buttonUP, BUTTON_UP_PIN, BUTTON2_ID);
  button_init(&buttonDOWN, BUTTON_DOWN_PIN, BUTTON3_ID);
  button_pressshort_set_callback((void *)button_press_short_callback);
  button_presslong_set_callback((void *)button_press_long_callback);

  xTaskCreatePinnedToCore(TaskButton, "TaskButton", 1024 * 10, NULL, 20, &TaskButton_handle, 1);
  xTaskCreatePinnedToCore(TaskOLEDDisplay, "TaskOLEDDisplay", 1024 * 16, NULL, 20, &TaskOLEDDisplay_handle, 1);
  xTaskCreatePinnedToCore(TaskKeypad, "TaskKeypad", 1024 * 4, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(TaskDHT11, "TaskDHT11", 1024 * 2, NULL, 10, &TaskDHT11_handle, 1);
  xTaskCreatePinnedToCore(TaskRFID, "TaskRFID", 1024 * 3, NULL, 12, &TaskRFID_handle, 1);
  xTaskCreatePinnedToCore(TaskAS608, "TaskAS608", 1024 * 3, NULL, 8, &TaskAS608_handle, 1);

  // connectAPMode();
  // Kết nối wifi
  connectSTA();
}

void loop()
{
  vTaskDelete(NULL);
}

//---------------------------------------- Task to the AS608 sentor data ----------------------------------
void TaskAS608(void *pvParameters)
{
  // Khởi tạo UART cho cảm biến vân tay
  mySerial.begin(57600, SERIAL_8N1, AS608_RX, AS608_TX);
  vTaskDelay(100 / portTICK_PERIOD_MS); // Đảm bảo UART được khởi tạo

  // Biến quản lý thời gian và bộ đếm
  unsigned long lastFingerprintTime = millis(); // Thời gian cuối cùng nhận được dấu vân tay
  uint8_t reactivationCounter = 0;              // Bộ đếm để bật lại cảm biến
  bool disableMessagePrinted = false;           // Cờ để in thông báo tắt chỉ một lần

  // Khởi tạo cảm biến vân tay
  finger.begin(57600);
  while (!finger.verifyPassword())
  {
    Serial.println("Không tìm thấy cảm biến vân tay :(");
    vTaskDelay(10000 / portTICK_PERIOD_MS); // Chờ 10 giây trước khi thử lại
    finger.begin(57600);                    // Thử khởi tạo lại cảm biến
  }
  Serial.println("Đã tìm thấy cảm biến vân tay!");

  // Đọc và in ra các thông số của cảm biến
  Serial.println(F("Đang đọc các thông số của cảm biến"));
  finger.getParameters();
  Serial.print(F("Trạng thái: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Dung lượng: "));
  Serial.println(finger.capacity);
  Serial.print(F("Mức độ bảo mật: "));
  Serial.println(finger.security_level);
  Serial.print(F("Địa chỉ thiết bị: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Độ dài gói tin: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Tốc độ baud: "));
  Serial.println(finger.baud_rate);

  finger.getTemplateCount();
  Serial.print(F("Cảm biến chứa: "));
  Serial.print(finger.templateCount);
  Serial.println(" mẫu vân tay");

  // Đặt lại thời gian sau khi cảm biến được khởi tạo thành công
  lastFingerprintTime = millis();

  while (1)
  { // Vòng lặp chính cho task
    if (statusAS608 == DISABLE)
    {
      // Gửi lệnh sleep để đưa cảm biến vào chế độ ngủ
      sendSleepCommand();

      // Chỉ in thông báo tắt một lần
      if (!disableMessagePrinted)
      {
        Serial.println("AS608 đã bị tắt. Vui lòng chạm vào cảm biến hai lần để bật lại.");
        disableMessagePrinted = true;
      }

      unsigned long disableStartTime = millis(); // Bắt đầu tính thời gian khi vào DISABLE
      reactivationCounter = 0;                   // Đặt lại bộ đếm
      bool success = false;

      while (millis() - disableStartTime < 10000) // Chờ tối đa 10 giây
      {
        // Gửi lệnh wake để đánh thức cảm biến
        sendWakeCommand();
        vTaskDelay(100 / portTICK_PERIOD_MS); // Chờ cảm biến sẵn sàng

        // Thử kết nối lại với cảm biến
        if (!finger.verifyPassword())
        {
          Serial.println("Không thể kết nối lại với cảm biến vân tay.");
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          continue;
        }

        // Chờ dấu vân tay
        uint8_t fingerprintID = getFingerprintID();
        if (fingerprintID > 0)
        {
          Serial.println("Đã nhận diện dấu vân tay.");
          reactivationCounter++;

          if (reactivationCounter >= 1)
          {
            success = true;
            break;
          }
        }
        else
        {
          Serial.println("Không nhận diện được dấu vân tay. Vui lòng thử lại.");
        }

        // Đưa cảm biến vào chế độ ngủ sau khi kiểm tra
        sendSleepCommand();

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Thêm delay để tránh chạy quá nhanh
      }

      // Kiểm tra nếu thành công
      if (success)
      {
        // Gửi lệnh wake để đánh thức cảm biến
        sendWakeCommand();
        vTaskDelay(200 / portTICK_PERIOD_MS); // Chờ cảm biến sẵn sàng

        if (finger.verifyPassword())
        {
          Serial.println("AS608 đã được bật lại.");
          statusAS608 = ENABLE; // Bật lại cảm biến
          reactivationCounter = 0;
          lastFingerprintTime = millis(); // Reset thời gian
          disableMessagePrinted = false;  // Reset cờ thông báo tắt
        }
        else
        {
          Serial.println("Không thể xác minh mật khẩu cảm biến sau khi bật lại.");
          statusAS608 = DISABLE;
        }
      }
      else
      {
        Serial.println("Không thể bật lại AS608 trong thời gian giới hạn. Nó vẫn đang bị tắt.");
        reactivationCounter = 0; // Đặt lại bộ đếm để thử lại lần sau
      }

      vTaskDelay(100 / portTICK_PERIOD_MS); // Tránh task chạy quá nhanh
      continue;                             // Bỏ qua phần còn lại của vòng lặp khi AS608 đang tắt
    }

    // AS608 đang hoạt động bình thường
    uint8_t fingerprintID = getFingerprintID();
    if (fingerprintID > 0)
    {
      // Dấu vân tay hợp lệ được phát hiện
      Serial.println("Đã nhận diện dấu vân tay. Bật DOOR...");
      controlLock(DISABLE);                  // Mở khóa (Relay)
      buzzerBeep(2);                         // Beep 2 lần
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Giữ Relay mở trong 5 giây
      controlLock(ENABLE);                   // Đóng khóa (Relay)
      lastFingerprintTime = millis();        // Reset thời gian
      disableMessagePrinted = false;         // Reset cờ thông báo tắt
    }

    // Kiểm tra nếu đã quá 10 giây mà không có dấu vân tay nào được phát hiện
    if (millis() - lastFingerprintTime > 10000)
    {
      // Tắt AS608
      statusAS608 = DISABLE;
      Serial.println("Không phát hiện dấu vân tay trong 10 giây. Đã tắt AS608.");
    }

    vTaskDelay(100 / portTICK_PERIOD_MS); // Thêm delay để tránh task chạy quá nhanh
  }
}

// Hàm để gửi lệnh sleep đến cảm biến
void sendSleepCommand()
{
  uint8_t sleepPacket[] = {
      0xEF, 0x01,             // Start code
      0xFF, 0xFF, 0xFF, 0xFF, // Device ID
      0x01,                   // Packet Identifier (Command packet)
      0x00, 0x03,             // Packet Length (3 bytes)
      0x33,                   // Instruction code (Sleep)
      0x00, 0x37              // Checksum (đã tính sẵn)
  };
  // Nếu cần tính lại checksum:
  // uint16_t checksum = 0x01 + 0x00 + 0x03 + 0x33;
  // sleepPacket[9] = (checksum >> 8) & 0xFF;
  // sleepPacket[10] = checksum & 0xFF;

  mySerial.write(sleepPacket, sizeof(sleepPacket));
}

// Hàm để gửi lệnh wake đến cảm biến
void sendWakeCommand()
{
  // Thường thì cảm biến sẽ tự động thức dậy khi nhận được dữ liệu hoặc khi có hoạt động
  // Nếu cần, bạn có thể gửi lệnh dummy để đánh thức cảm biến
  // Ở đây chúng ta có thể gửi một lệnh empty hoặc lệnh verify password

  uint8_t verifyPacket[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                            0x01, 0x00, 0x07, 0x13,
                            0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x1B};
  mySerial.write(verifyPacket, sizeof(verifyPacket));
}

// Hàm để lấy ID dấu vân tay, trả về 0 nếu không tìm thấy
uint8_t getFingerprintID()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
  {
    // Không nhận được dấu vân tay hợp lệ hoặc lỗi giao tiếp
    switch (p)
    {
    case FINGERPRINT_NOFINGER:
      // Không phát hiện ngón tay
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi giao tiếp");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Lỗi chụp ảnh");
      break;
    default:
      Serial.println("Lỗi không xác định");
      break;
    }
    return 0;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
  {
    // Chuyển đổi hình ảnh thất bại
    switch (p)
    {
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Hình ảnh quá lộn xộn");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi giao tiếp");
      break;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Không tìm thấy các đặc trưng của dấu vân tay");
      break;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Hình ảnh không hợp lệ");
      break;
    default:
      Serial.println("Lỗi không xác định");
      break;
    }
    return 0;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    Serial.print("Đã tìm thấy ID #");
    Serial.print(finger.fingerID);
    Serial.print(" với độ tin cậy là ");
    Serial.println(finger.confidence);
    return finger.fingerID; // Trả về ID của dấu vân tay
  }
  else
  {
    switch (p)
    {
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi giao tiếp");
      break;
    case FINGERPRINT_NOTFOUND:
      Serial.println("Không tìm thấy dấu vân tay phù hợp");
      break;
    default:
      Serial.println("Lỗi không xác định");
      break;
    }
    return 0;
  }
}

//---------------------------------------------------------------------------------------------------------

//-----------------------------------------Task to the RFID sentor data ------------------------------------
void TaskRFID(void *pvParameters)
{
  // Khởi tạo RFID trong task
  initRFID();
  lastCardDetectedTime = millis(); // Khởi tạo thời gian

  while (1)
  {
    if (statusRFID == DISABLE)
    {
      // RFID đã bị tắt, chờ người dùng quẹt thẻ hợp lệ hai lần để bật lại

      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
      {
        // Kiểm tra xem thẻ có hợp lệ không
        bool isAllowed = false;
        for (int i = 0; i < allowedUIDCount; i++)
        {
          bool match = true;
          for (byte j = 0; j < 4; j++)
          {
            if (mfrc522.uid.uidByte[j] != allowedUIDs[i][j])
            {
              match = false;
              break;
            }
          }
          if (match)
          {
            isAllowed = true;
            break;
          }
        }

        if (isAllowed)
        {
          reactivationCounter++;
          Serial.print("Đã quẹt thẻ hợp lệ ");
          Serial.print(reactivationCounter);
          Serial.println(" lần.");

          if (reactivationCounter >= 1)
          {
            // Kích hoạt lại RFID
            statusRFID = ENABLE;
            reactivationCounter = 0;
            Serial.println("Đã bật lại RFID.");
            lastCardDetectedTime = millis(); // Đặt lại thời gian
          }
        }
        else
        {
          reactivationCounter = 0; // Nếu thẻ không hợp lệ, đặt lại bộ đếm
          Serial.println("Thẻ không hợp lệ. Vui lòng sử dụng thẻ hợp lệ để bật lại RFID.");
        }

        // Dừng giao tiếp với thẻ
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
      }

      vTaskDelay(100 / portTICK_PERIOD_MS); // Tránh task chạy quá nhanh
      continue;                             // Bỏ qua phần còn lại của vòng lặp khi RFID đang tắt
    }

    // RFID đang hoạt động bình thường
    // Kiểm tra xem có thẻ mới được đưa vào không
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
      lastCardDetectedTime = millis(); // Cập nhật thời gian thẻ được phát hiện

      // So sánh UID của thẻ với danh sách UID hợp lệ
      bool isAllowed = false;
      for (int i = 0; i < allowedUIDCount; i++)
      {
        bool match = true;
        for (byte j = 0; j < 4; j++)
        {
          if (mfrc522.uid.uidByte[j] != allowedUIDs[i][j])
          {
            match = false;
            break;
          }
        }
        if (match)
        {
          isAllowed = true;
          break;
        }
      }

      if (isAllowed)
      {
        // Nếu UID khớp, bật đèn xanh
        Serial.println("Thẻ hợp lệ. Open door.");
        controlLock(DISABLE);
        buzzerBeep(2);
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Relay trong 1 giây
        controlLock(ENABLE);
      }
      else
      {
        // Nếu UID không khớp, bật đèn đỏ
        Serial.println("Thẻ không hợp lệ.");
      }

      // Dừng giao tiếp với thẻ
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }

    // Kiểm tra nếu đã quá 10 giây mà không có thẻ nào được phát hiện
    if (millis() - lastCardDetectedTime > 10000)
    {
      // Tắt RFID
      statusRFID = DISABLE;
      Serial.println("Không phát hiện thẻ trong 10 giây. Đã tắt RFID.");
    }

    vTaskDelay(100 / portTICK_PERIOD_MS); // Thêm delay để tránh task chạy quá nhanh
  }
}

// Hàm khởi tạo RFID
void initRFID()
{
  mfrc522.PCD_Init();
  Serial.println("Đưa thẻ của bạn vào gần đầu đọc...");
  Serial.println();
  mfrc522.PCD_AntennaOn(); // Bật ăng-ten RFID nếu cần
}

//----------------------------------------- Task to read DHT11 sensor data
// Task to read DHT11 sensor data with modifications to stop warnings after 5 unsuccessful attempts and re-enable detection when reconnected
void TaskDHT11(void *pvParameters)
{
  const int maxErrorCount = 5;   // Maximum number of consecutive errors before stopping
  int errorCount = 0;            // Counter for consecutive errors
  bool readingEnabled = true;    // Flag to enable/disable reading
  bool detectionStopped = false; // Flag to indicate if detection is stopped after max errors

  while (true)
  {
    if (readingEnabled)
    {
      int humi = dht.readHumidity();
      float temp = dht.readTemperature();

      if (isnan(humi) || isnan(temp))
      {
        // If an error occurs, increment the error counter
        Serial.println(F("Failed to read from DHT sensor!"));
        dht11ReadOK = false;
        errorCount++;

        // Stop reading after reaching max error count
        if (errorCount >= maxErrorCount)
        {
          Serial.println("Too many errors, stopping DHT11 reading");
          readingEnabled = false;  // Disable further reading
          detectionStopped = true; // Indicate detection is stopped
        }
      }
      else if (humi <= 100 && temp < 100)
      {
        dht11ReadOK = true;
        humiValue = humi;
        tempValue = temp;

        Serial.print(F("Humidity: "));
        Serial.print(humiValue);
        Serial.print(F("%  Temperature: "));
        Serial.print(tempValue);
        Serial.print(F("°C "));
        Serial.println();

        if (tempValue < EtempThreshold1 || tempValue > EtempThreshold2)
          warningTempState = NORMAL;
        else
          warningTempState = HAPPY;
        if (humiValue < EhumiThreshold1 || humiValue > EhumiThreshold2)
          warningHumiState = NORMAL;
        else
          warningHumiState = HAPPY;
        errorCount = 0; // Reset error count on successful read
      }

      delay(10000); // Wait 3 seconds before the next reading
    }
    else
    {
      // Check periodically if DHT11 reading can be re-enabled
      vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait 1 second before checking again

      // If detection was stopped, re-enable reading if a valid value is detected
      int humi = dht.readHumidity();
      float temp = dht.readTemperature();

      if (detectionStopped && !isnan(humi) && !isnan(temp))
      {
        errorCount = 0;           // Reset error count
        readingEnabled = true;    // Re-enable DHT11 reading
        detectionStopped = false; // Reset detection stopped flag
        Serial.println("DHT11 sensor re-enabled");
      }
    }
  }
}
//---------------------------------------------KEYPAD----------------------------

void TaskKeypad(void *pvParameters)
{
  while (1)
  {
    switch (modeRun)
    {
    case IDLE_MODE:
      allowAccess = 0;
      if (notiOne == 1)
      {
        Blynk.virtualWrite(V0, DISABLE);
        digitalWrite(LED, LED_OFF);
        Serial.println("Khoa cua");
        controlLock(ENABLE);
        notiOne = 0;
        screenOLED = SCREEN1;
        enableShow3 = DISABLE;
        enableShow2 = DISABLE;
        enableShow1 = ENABLE;
        OLED_STRING1 = "Xin chao";
        OLED_STRING2 = "Hi my friend";
        // SCREEN_SHOW Nhập mật khẩu + ****

        strcpy(starkey, "        ");
      }
      tick(); // Gọi hàm tick() để cập nhật trạng thái bàn phím
      while (!keyEventQueue.isEmpty())
      {
        keypadEvent e = read();
        if (e.EVENT == KEY_JUST_RELEASED)
        {

          buzzerBeep(1);

          if ((char)e.KEY == '#')
          {
            countKey = 0;
            modeRun = IDLE_MODE;
            Serial.println("cancel");
            strcpy(starkey, "        ");
            notiOne = 1;
          }
          else
          {
            passwordEnter[countKey] = (char)e.KEY;
            Serial.print((char)e.KEY);
            starkey[countKey * 2] = '*';
            starkey[countKey * 2 + 1] = ' ';
            countKey++;
            screenOLED = SCREEN2;
            enableShow3 = DISABLE;
            enableShow2 = ENABLE;
            enableShow1 = DISABLE;
            // SCREEN_SHOW Nhập mật khẩu + * * * *
            OLED_STRING1 = "Nhap mat khau";
            OLED_STRING2 = starkey;
          }
        }
      }
      if (countKey > 3)
      {
        passwordEnter[4] = '\0';
        delay(50);
        Serial.println(passwordEnter);
        timeMillisAUTO = millis();
        if (strcmp(passwordEnter, password) == 0)
        {
          buzzerBeep(3);
          allowAccess = 1;
          Serial.println("CORRECT PASSWORD");
          // SCREEN_SHOW Mật khẩu đúng, mời vào
          // oledDisplayPass("MAT KHAU DUNG","MOI VAO");
          enableShow3 = DISABLE;
          enableShow2 = ENABLE;
          enableShow1 = DISABLE;
          screenOLED = SCREEN2;
          OLED_STRING1 = "Mat khau dung";
          OLED_STRING2 = "Moi vao";
          strcpy(starkey, "        ");
          digitalWrite(LED, LED_ON);
          Serial.println("MỞ khóa");
          controlLock(DISABLE);
          modeRun = WAIT_CLOSE;
        }
        else
        {

          // SCREEN_SHOW Màn hình, nhập sai mật khẩu, còn n lần thử
          enableShow3 = DISABLE;
          enableShow2 = ENABLE;
          enableShow1 = DISABLE;
          screenOLED = SCREEN2;
          OLED_STRING1 = "Mat khau sai";
          OLED_STRING2 = "Con " + String((EnumberEnterWrong - countError - 1)) + " lan";
          buzzerWarningBeep(1000);
          if (autoWarning == 1)
            Blynk.logEvent("auto_warning", "Cảnh báo đột nhập");

          strcpy(starkey, "        ");
          countError++;
          notiOne = 1;
          delay(1000);
          if (countError >= EnumberEnterWrong)
            modeRun = LOCK_1MIN;
        }
        countKey = 0;
      }
      break;
    case WAIT_CLOSE:
      // auto lock door
      countError = 0;
      notiOne = 1;
      if (millis() - timeMillisAUTO > EtimeOpenDoor * 1000)
      {
        timeMillisAUTO = millis();
        countKey = 0;
        delay(20);
        modeRun = IDLE_MODE;
      }
      tick(); // Gọi hàm tick() để cập nhật trạng thái bàn phím
      while (!keyEventQueue.isEmpty())
      {
        keypadEvent e = read();
        // đợi k cho về idle mode
        timeMillisAUTO = millis();
        // COI_beep();
        if (e.EVENT == KEY_JUST_RELEASED)
        {
          buzzerBeep(3);
          keyMode = (char)e.KEY;
          Serial.println((char)e.KEY);
          if (keyMode == '*')
          {
            if (EenableChangePass == ENABLE)
            {
              Serial.println("mode doi pass");
              Serial.println("khoa cua");
              // oledDisplayPass("NHAP MAT KHAU MOI","");
              // SCREEN_SHOW Nhập mật khẩu mới
              enableShow3 = ENABLE;
              enableShow2 = DISABLE;
              enableShow1 = DISABLE;
              screenOLED = SCREEN3;
              OLED_STRING1 = "Mat khau moi";
              OLED_STRING2 = "";

              controlLock(ENABLE);
              blinkLED(3);

              strcpy(starkey, "        ");
              modeRun = CHANGE_PASSWORD;
            }
            else
            {
              Serial.println("Khong cho phep doi mat khau");
              // SCREEN_SHOW Nhập mật khẩu mới
              enableShow3 = ENABLE;
              enableShow2 = DISABLE;
              enableShow1 = DISABLE;
              screenOLED = SCREEN3;
              OLED_STRING1 = "Khong the";
              OLED_STRING2 = "doi mat khau";
              controlLock(ENABLE);
              buzzerWarningBeep(1000);
              strcpy(starkey, "        ");
              delay(1000);
              modeRun = IDLE_MODE;
            }
          }
          else
            modeRun = IDLE_MODE;
        }
      }
      break;
    case CHANGE_PASSWORD:
      changePasswordFunc();
      break;
    case LOCK_1MIN:
      notiOne = 1;
      Serial.println("thu lai sau 1 phut");
      buzzerWarningBeep(2000);
      for (int i = EtimeLock; i >= 0; i--)
      {
        enableShow3 = DISABLE;
        enableShow2 = DISABLE;
        enableShow1 = ENABLE;
        screenOLED = SCREEN1;
        OLED_STRING1 = "Thu lai sau";
        OLED_STRING2 = String(i) + " giay";

        // Màn hình thử lại sau 60s
        Serial.print("Thu lai sau ");
        Serial.print(i);
        Serial.println(" s");
        delay(1000);
      }
      countError = 0;
      modeRun = IDLE_MODE;
      break;
    }
    delay(10);
  }
}
//--------------------------- Hàm Control Lock --------------------------------
void controlLock(int state)
{
  digitalWrite(RELAY, state);
  Blynk.virtualWrite(V0, state);
}
//---------------------- Hàm đổi mật khẩu ----------------------------------------------
void changePasswordFunc()
{
  switch (changePassword)
  {
  case ENTER_PASS_1:
    tick(); // Gọi hàm tick() để cập nhật trạng thái bàn phím
    while (!keyEventQueue.isEmpty())
    {
      keypadEvent e = read();
      // đợi k cho về idle mode
      timeMillisAUTO = millis();

      // COI_beep();
      if (e.EVENT == KEY_JUST_RELEASED)
      {
        buzzerBeep(1);
        if ((char)e.KEY == '#')
        {
          countKey = 0;
          modeRun = IDLE_MODE;
          Serial.println("cancel");
          notiOne = 1;
        }
        else
        {
          passwordChange1[countKey] = (char)e.KEY;
          Serial.println((char)e.KEY);
          starkey[countKey * 2] = '*';
          starkey[countKey * 2 + 1] = ' ';
          countKey++;
          // oledDisplayPass("NHAP MAT KHAU",starkey);
          // SCREEN_SHOW Nhập mật khẩu
          screenOLED = SCREEN3;
          OLED_STRING1 = "Mat khau moi";
          OLED_STRING2 = starkey;
        }
      }
    }
    if (countKey > 3)
    {
      delay(200);
      changePassword = ENTER_PASS_2;
      buzzerBeep(2);
      countKey = 0;
      // SCREEN_SHOW Nhập lại mật khẩu
      screenOLED = SCREEN3;
      OLED_STRING1 = "Nhap lai";
      OLED_STRING2 = "";
      memset(starkey, 0, sizeof(starkey));
    }
    break;
  case ENTER_PASS_2:
    tick(); // Gọi hàm tick() để cập nhật trạng thái bàn phím
    while (!keyEventQueue.isEmpty())
    {
      keypadEvent e = read();
      // đợi k cho về idle mode
      timeMillisAUTO = millis();
      // COI_beep();
      if (e.EVENT == KEY_JUST_RELEASED)
      {
        buzzerBeep(1);
        if ((char)e.KEY == '#')
        {
          countKey = 0;
          modeRun = IDLE_MODE;
          Serial.println("cancel");
          notiOne = 1;
        }
        else
        {
          passwordChange2[countKey] = (char)e.KEY;
          // Serial.println((char)e.bit.KEY);
          starkey[countKey * 2] = '*';
          starkey[countKey * 2 + 1] = ' ';
          countKey++;

          // SCREEN_SHOW Nhập lại mật khẩu + * * * *
          screenOLED = SCREEN3;
          OLED_STRING1 = "Nhap lai";
          OLED_STRING2 = starkey;
        }
      }
    }
    if (countKey > 3)
    {
      memset(starkey, 0, sizeof(starkey));
      delay(50);
      if (strcmp(passwordChange1, passwordChange2) == 0)
      {

        // SCREEN_SHOW Đổi mật khẩu + thành công
        screenOLED = SCREEN3;
        OLED_STRING1 = "Doi mat khau";
        OLED_STRING2 = "Thanh cong";
        memset(starkey, 0, sizeof(starkey));
        buzzerBeep(5);
        blinkLED(3);
        passwordChange2[4] = '\0';
        memcpy(password, passwordChange1, 5);
        Serial.print("new pass");
        Serial.println(password);
        Blynk.logEvent("auto_warning", "Cảnh báo đổi mật khẩu : " + String(password));
        // Blynk.logEvent("auto_warning","Cảnh báo đổi mật khẩu ");
        // luu vao flash
        uint32_t valueCV = convertPassToNumber();
        EpassDoor = valueCV;
        savePWtoEEP(valueCV);
        savePASStoBUFF();
        Serial.println("CHANGE SUCCESSFUL ");
        delay(2000);
        changePassword = ENTER_PASS_1;
        modeRun = IDLE_MODE;
      }
      else
      {
        blinkLED(1);
        // SCREEN_SHOW Mật khẩu sai, nhập lại
        screenOLED = SCREEN3;
        OLED_STRING1 = "Mat khau sai";
        OLED_STRING2 = "Nhap lai";
        Serial.println("NOT CORRECT");
        buzzerWarningBeep(1000);
        changePassword = ENTER_PASS_1;
        strcpy(starkey, "        ");

        // SCREEN_SHOW Nhập mật khẩu mới
        // oledDisplayPass("NHAP MAT KHAU MOI","");
        OLED_STRING1 = "Mat khau moi";
        OLED_STRING2 = "";
      }
      countKey = 0;
    }
    break;
  }
}

// Xóa 1 ô hình chữ nhật từ tọa độ (x1,y1) đến (x2,y2)
void clearRectangle(int x1, int y1, int x2, int y2)
{
  for (int i = y1; i < y2; i++)
  {
    oled.drawLine(x1, i, x2, i, 0);
  }
}

void clearOLED()
{
  oled.clearDisplay();
  oled.display();
}

int countSCREEN9 = 0;
// Task hiển thị OLED
void TaskOLEDDisplay(void *pvParameters)
{
  while (1)
  {
    switch (screenOLED)
    {
    case SCREEN0: // Hiệu ứng khởi động
      for (int j = 0; j < 3; j++)
      {
        for (int i = 0; i < FRAME_COUNT_loadingOLED; i++)
        {
          oled.clearDisplay();
          oled.drawBitmap(32, 0, loadingOLED[i], FRAME_WIDTH_64, FRAME_HEIGHT_64, 1);
          oled.display();
          delay(FRAME_DELAY / 4);
        }
      }
      screenOLED = SCREEN4;
      break;
    case SCREEN1: // Man hinh chao
      for (int j = 0; j < 2 && enableShow1 == ENABLE; j++)
      {
        for (int i = 0; i < FRAME_COUNT_hiOLED && enableShow1 == ENABLE; i++)
        {
          oled.clearDisplay();
          oled.setTextSize(1);
          oled.setCursor(40, 22);
          oled.print(OLED_STRING1);
          oled.setTextSize(1);
          oled.setCursor(40, 42);
          oled.print(OLED_STRING2);
          clearRectangle(0, 0, 32, 64);
          oled.drawBitmap(0, 16, hiOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
          oled.display();
          delay(FRAME_DELAY);
        }
        oled.display();
        delay(100);
      }
      break;
    case SCREEN2: // Man hinh nhap key
      for (int j = 0; j < 2 && enableShow2 == ENABLE; j++)
      {
        for (int i = 0; i < FRAME_COUNT_keyOLED && enableShow2 == ENABLE; i++)
        {
          oled.clearDisplay();
          oled.setTextSize(1);
          oled.setCursor(40, 22);
          oled.print(OLED_STRING1);
          oled.setTextSize(1);
          oled.setCursor(40, 42);
          oled.print(OLED_STRING2);
          clearRectangle(0, 0, 32, 64);
          oled.drawBitmap(0, 16, keyOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
          oled.display();
          delay(FRAME_DELAY);
        }
        oled.display();
        delay(100);
      }
      break;
    case SCREEN3: // Man hinh doi mat khau
      for (int j = 0; j < 2 && enableShow3 == ENABLE; j++)
      {
        for (int i = 0; i < FRAME_COUNT_lockOpenOLED && enableShow3 == ENABLE; i++)
        {
          oled.clearDisplay();

          oled.setTextSize(1);
          oled.setCursor(40, 22);
          oled.print(OLED_STRING1);
          oled.setTextSize(1);
          oled.setCursor(40, 42);
          oled.print(OLED_STRING2);
          clearRectangle(0, 0, 32, 64);
          oled.drawBitmap(0, 16, lockOpenOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
          oled.display();
          delay(FRAME_DELAY);
        }
        oled.display();
        delay(100);
      }
      break;
    case SCREEN4: // Đang kết nối Wifi
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 5);
      oled.print("WIFI");
      oled.setTextSize(1.5);
      oled.setCursor(40, 17);
      oled.print("Dang ket noi..");

      for (int i = 0; i < FRAME_COUNT_wifiOLED; i++)
      {
        clearRectangle(0, 0, 32, 32);
        oled.drawBitmap(0, 0, wifiOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
        oled.display();
        delay(FRAME_DELAY);
      }
      break;
    case SCREEN5: // Kết nối wifi thất bại
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 5);
      oled.print("WIFI");
      oled.setTextSize(1.5);
      oled.setCursor(40, 17);
      oled.print("Mat ket noi.");
      oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
      oled.drawLine(31, 0, 0, 31, 1);
      oled.drawLine(32, 0, 0, 32, 1);
      oled.display();
      delay(2000);
      screenOLED = SCREEN9;
      break;
    case SCREEN6: // Đã kết nối Wifi, đang kết nối Blynk
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 5);
      oled.print("WIFI");
      oled.setTextSize(1.5);
      oled.setCursor(40, 17);
      oled.print("Da ket noi.");
      oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);

      oled.setTextSize(1);
      oled.setCursor(40, 34);
      oled.print("BLYNK");
      oled.setTextSize(1.5);
      oled.setCursor(40, 51);
      oled.print("Dang ket noi..");

      for (int i = 0; i < FRAME_COUNT_blynkOLED; i++)
      {
        clearRectangle(0, 32, 32, 64);
        oled.drawBitmap(0, 32, blynkOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
        oled.display();
        delay(FRAME_DELAY);
      }

      break;
    case SCREEN7: // Đã kết nối Wifi, Đã kết nối Blynk
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 5);
      oled.print("WIFI");
      oled.setTextSize(1.5);
      oled.setCursor(40, 17);
      oled.print("Da ket noi.");
      oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);

      oled.setTextSize(1);
      oled.setCursor(40, 34);
      oled.print("BLYNK");
      oled.setTextSize(1.5);
      oled.setCursor(40, 51);
      oled.print("Da ket noi.");
      oled.drawBitmap(0, 32, blynkOLED[FRAME_COUNT_wifiOLED / 2], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
      oled.display();
      delay(2000);
      screenOLED = SCREEN1;
      break;
    case SCREEN8: // Đã kết nối Wifi, Mat kết nối Blynk
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 5);
      oled.print("WIFI");
      oled.setTextSize(1.5);
      oled.setCursor(40, 17);
      oled.print("Da ket noi.");
      oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);

      oled.setTextSize(1);
      oled.setCursor(40, 34);
      oled.print("BLYNK");
      oled.setTextSize(1.5);
      oled.setCursor(40, 51);
      oled.print("Mat ket noi.");
      oled.drawBitmap(0, 32, blynkOLED[FRAME_COUNT_wifiOLED / 2], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
      oled.drawLine(31, 32, 0, 63, 1);
      oled.drawLine(32, 32, 0, 64, 1);
      oled.display();
      delay(2000);
      screenOLED = SCREEN9;
      break;
    case SCREEN9: // Cai đặt 192.168.4.1
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 5);
      oled.setTextSize(1);
      oled.print("Ket noi Wifi:");
      oled.setCursor(40, 17);
      oled.setTextSize(1);
      oled.print("ESP32_IOT");
      oled.setCursor(40, 38);
      oled.print("Dia chi IP:");
      oled.setCursor(40, 50);
      oled.print("192.168.4.1");

      for (int i = 0; i < FRAME_COUNT_settingOLED; i++)
      {
        clearRectangle(0, 0, 32, 64);
        oled.drawBitmap(0, 16, settingOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
        oled.display();
        delay(FRAME_DELAY * 2);
      }
      countSCREEN9++;
      if (countSCREEN9 > 10)
      {
        countSCREEN9 = 0;
        screenOLED = SCREEN1;
        enableShow1 = ENABLE;
      }

      break;
    case SCREEN10: // auto : on
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 20);
      oled.print(OLED_STRING3);
      oled.setTextSize(2);
      oled.setCursor(40, 32);
      oled.print("DISABLE");
      for (int i = 0; i < FRAME_COUNT_autoOnOLED; i++)
      {
        clearRectangle(0, 0, 32, 64);
        oled.drawBitmap(0, 16, autoOnOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
        oled.display();
        delay(FRAME_DELAY);
      }
      clearRectangle(40, 32, 128, 64);
      oled.setCursor(40, 32);
      oled.print("ENABLE");
      oled.display();
      delay(2000);
      screenOLED = SCREEN1;
      enableShow1 = ENABLE;
      break;
    case SCREEN11: // auto : off
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 20);
      oled.print(OLED_STRING3);
      oled.setTextSize(2);
      oled.setCursor(40, 32);
      oled.print("ENABLE");
      for (int i = 0; i < FRAME_COUNT_autoOffOLED; i++)
      {
        clearRectangle(0, 0, 32, 64);
        oled.drawBitmap(0, 16, autoOffOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
        oled.display();
        delay(FRAME_DELAY);
      }
      clearRectangle(40, 32, 128, 64);
      oled.setCursor(40, 32);
      oled.print("DISABLE");
      oled.display();
      delay(2000);
      screenOLED = SCREEN1;
      enableShow1 = ENABLE;
      break;
    case SCREEN12: // gui du lieu len blynk
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(40, 20);
      oled.print("Gui du lieu");
      oled.setCursor(40, 32);
      oled.print("den BLYNK");
      for (int i = 0; i < FRAME_COUNT_sendDataOLED; i++)
      {
        clearRectangle(0, 0, 32, 64);
        oled.drawBitmap(0, 16, sendDataOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
        oled.display();
        delay(FRAME_DELAY);
      }
      delay(1000);
      screenOLED = SCREEN1;
      enableShow1 = ENABLE;
      break;
    case SCREEN13: // khoi dong lai
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setCursor(0, 20);
      oled.print("Khoi dong lai");
      oled.setCursor(0, 32);
      oled.print("Vui long doi ...");
      oled.display();
      break;
    default:
      delay(500);
      break;
    }
    delay(10);
  }
}
// lưu pass từ EEPROM vào mảng
void savePASStoBUFF()
{
  uint32_t c, d;
  c = EEPROM.read(250);
  d = EEPROM.read(251);
  if (c == 255 && d == 255)
  {
    memcpy(password, "1111", 4);
  }
  else
  {
    password[3] = (char)(d % 10 + 48);
    password[2] = (char)(d / 10 + 48);
    password[1] = (char)(c % 10 + 48);
    password[0] = (char)(c / 10 + 48);
  }
  Serial.println(password);
}
// chuyển mảng thành số
uint32_t convertPassToNumber()
{
  uint32_t valuee = ((uint32_t)password[3] - 48) + ((uint32_t)password[2] - 48) * 10 +
                    ((uint32_t)password[1] - 48) * 100 + ((uint32_t)password[0] - 48) * 1000;
  Serial.println(valuee);
  return valuee;
}

void savePWtoEEP(uint32_t valuePASS)
{
  uint32_t c, d;
  d = valuePASS / 100;
  c = valuePASS % 100;

  EEPROM.write(250, d);
  EEPROM.commit();
  EEPROM.write(251, c);
  EEPROM.commit();
  Serial.println(d);
  Serial.println(c);
}

//-----------------Kết nối STA wifi, chuyển sang wifi AP nếu kết nối thất bại -----------------------
void connectSTA()
{
  delay(5000);
  enableShow1 = DISABLE;
  if (Essid.length() > 1)
  {
    Serial.println(Essid);  // Print SSID
    Serial.println(Epass);  // Print Password
    Serial.println(Etoken); // Print token
    Etoken = Etoken.c_str();
    WiFi.begin(Essid.c_str(), Epass.c_str()); // c_str()
    int countConnect = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      if (countConnect++ == 15)
      {
        Serial.println("Ket noi Wifi that bai");
        Serial.println("Kiem tra SSID & PASS");
        Serial.println("Ket noi Wifi: ESP32 de cau hinh");
        Serial.println("IP: 192.168.4.1");
        screenOLED = SCREEN5;
        digitalWrite(BUZZER, ENABLE);
        delay(2000);
        digitalWrite(BUZZER, DISABLE);
        delay(3000);
        break;
      }
      // MODE đang kết nối wifi
      screenOLED = SCREEN4;
      delay(2000);
    }
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Da ket noi Wifi: ");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println((char *)Essid.c_str());

      // MODE wifi đã kết nối, đang kết nối blynk
      screenOLED = SCREEN6;
      delay(2000);
      strcpy(BLYNK_AUTH_TOKEN, Etoken.c_str());

      Blynk.config(BLYNK_AUTH_TOKEN);
      blynkConnect = Blynk.connect();
      if (blynkConnect == false)
      {
        screenOLED = SCREEN8;
        delay(2000);
        connectAPMode();
      }
      else
      {
        Serial.println("Da ket noi BLYNK");
        enableShow1 = ENABLE;
        // MODE đã kết nối wifi, đã kết nối blynk
        screenOLED = SCREEN7;
        delay(2000);
        xTaskCreatePinnedToCore(TaskBlynk, "TaskBlynk", 1024 * 16, NULL, 20, NULL, 1);
        timer.setInterval(1000L, myTimer);
        buzzerBeep(5);
        return;
      }
    }
    else
    {
      digitalWrite(BUZZER, ENABLE);
      delay(2000);
      digitalWrite(BUZZER, DISABLE);
      // MODE truy cập vào 192.168.4.1
      screenOLED = SCREEN9;
      connectAPMode();
    }
  }
}

//--------------------------- switch AP Mode ---------------------------
void connectAPMode()
{

  // Khởi tạo Wifi AP Mode, vui lòng kết nối wifi ESP32, truy cập 192.168.4.1
  WiFi.softAP(ssidAP, passwordAP);

  // Gửi trang HTML khi client truy cập 192.168.4.1
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  // Gửi data ban đầu đến clientgetDataFromClient
  server.on("/data_before", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = getJsonData();
    request->send(200, "application/json", json); });

  // Get data từ client
  server.on("/post_data", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    request->send(200, "text/plain", "SUCCESS");
    enableShow1 = DISABLE;
    screenOLED = SCREEN13;
    delay(5000);
    ESP.restart(); }, NULL, getDataFromClient);

  // (Nếu muốn hiển thị danh sách thẻ RFID trong AP Mode, THÊM route này)
  server.on("/rfid_cards", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    JSONVar jsonList;
    for (size_t i = 0; i < allowedUIDs.size(); i++) {
      JSONVar item;
      String uidHex = "";
      for (int j = 0; j < 4; j++) {
        if (j > 0) uidHex += " ";
        char buf[3];
        sprintf(buf, "%02X", allowedUIDs[i][j]);
        uidHex += buf;
      }
      item["uid"]   = uidHex;
      item["level"] = allowedUIDLevels[i];
      jsonList[i]   = item;
    }
    String out = JSON.stringify(jsonList);
    request->send(200, "application/json", out); });

  // Start server
  server.begin();
}

//------------------- Hàm đọc data từ client gửi từ HTTP_POST "/post_data" -------------------
void getDataFromClient(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  Serial.print("get data : ");
  Serial.println((char *)data);
  JSONVar myObject = JSON.parse((char *)data);
  if (myObject.hasOwnProperty("ssid"))
    Essid = (const char *)myObject["ssid"];
  if (myObject.hasOwnProperty("pass"))
    Epass = (const char *)myObject["pass"];
  if (myObject.hasOwnProperty("token"))
    Etoken = (const char *)myObject["token"];
  if (myObject.hasOwnProperty("passDoor"))
    EpassDoor = (int)myObject["passDoor"];
  if (myObject.hasOwnProperty("timeOpenDoor"))
    EtimeOpenDoor = (int)myObject["timeOpenDoor"];
  if (myObject.hasOwnProperty("numberEnterWrong"))
    EnumberEnterWrong = (int)myObject["numberEnterWrong"];
  if (myObject.hasOwnProperty("timeLock"))
    EtimeLock = (int)myObject["timeLock"];

  writeEEPROM();
  savePWtoEEP(EpassDoor);
  savePASStoBUFF();
}

// ------------ Hàm in các giá trị cài đặt ------------
void printValueSetup()
{
  Serial.print("ssid = ");
  Serial.println(Essid);
  Serial.print("pass = ");
  Serial.println(Epass);
  Serial.print("token = ");
  Serial.println(Etoken);
  Serial.print("passDoor = ");
  Serial.println(EpassDoor);
  Serial.print("timeOpenDoor = ");
  Serial.println(EtimeOpenDoor);
  Serial.print("numberEnterWrong = ");
  Serial.println(EnumberEnterWrong);
  Serial.print("timeLock = ");
  Serial.println(EtimeLock);
  Serial.print("enableChangePass = ");
  Serial.println(EenableChangePass);
  Serial.print("tempThreshold1 = ");
  Serial.println(EtempThreshold1);
  Serial.print("tempThreshold2 = ");
  Serial.println(EtempThreshold2);
  Serial.print("humiThreshold1 = ");
  Serial.println(EhumiThreshold1);
  Serial.print("humiThreshold2 = ");
  Serial.println(EhumiThreshold2);

  Serial.print("autoWarning = ");
  Serial.println(autoWarning);
}

//-------- Hàm tạo biến JSON để gửi đi khi có request HTTP_GET "/" --------
String getJsonData()
{
  JSONVar myObject;
  myObject["ssid"] = Essid;
  myObject["pass"] = Epass;
  myObject["token"] = Etoken;
  myObject["passDoor"] = EpassDoor;
  myObject["timeOpenDoor"] = EtimeOpenDoor;
  myObject["numberEnterWrong"] = EnumberEnterWrong;
  myObject["timeLock"] = EtimeLock;

  String jsonData = JSON.stringify(myObject);
  return jsonData;
}

//-------------------------------------------------------------------------------
//--------------------------------Task Blynk-------------------------------------

//------------------------- check autoWarning from BLYNK  -----------------------
BLYNK_WRITE(V3)
{
  buzzerBeep(2);
  Blynk.logEvent("check_data", "Mật khẩu: " + String(password));
}
BLYNK_WRITE(V2)
{
  enableShow1 = DISABLE;
  EenableChangePass = param.asInt();
  buzzerBeep(2);
  EEPROM.write(204, EenableChangePass);
  EEPROM.commit();
  Serial.print("EenableChangePass : ");
  Serial.println(EenableChangePass);
  OLED_STRING3 = "Doi mat khau";
  if (EenableChangePass == 0)
    screenOLED = SCREEN11;
  else
    screenOLED = SCREEN10;
}
BLYNK_WRITE(V1)
{
  enableShow1 = DISABLE;
  autoWarning = param.asInt();
  buzzerBeep(1);
  EEPROM.write(210, autoWarning);
  EEPROM.commit();
  OLED_STRING3 = "Canh bao";
  if (autoWarning == 0)
    screenOLED = SCREEN11;
  else
    screenOLED = SCREEN10;
}
void myTimer()
{
  Blynk.virtualWrite(V4, tempValue);
  Blynk.virtualWrite(V5, humiValue);
}

//------------------------- check autoWarning from BLYNK  -----------------------
BLYNK_WRITE(V0)
{
  int state = param.asInt();
  if (state == 1)
  {
    Serial.println("open door");
    controlLock(DISABLE);
    buzzerBeep(3);
    delay(EtimeOpenDoor * 1000);
    controlLock(ENABLE);
  }
}
//---------------------------Task TaskSwitchAPtoSTA---------------------------
void TaskBlynk(void *pvParameters)
{
  Blynk.virtualWrite(V1, autoWarning);
  Blynk.virtualWrite(V2, EenableChangePass);
  while (1)
  {
    Blynk.run();
    timer.run();
    delay(10);
  }
}

/*
 * Các hàm liên quan đến lưu dữ liệu cài đặt vào EEPROM
 */
//--------------------------- Read Eeprom  --------------------------------
void readEEPROM()
{
  for (int i = 0; i < 32; ++i) // Reading SSID
    Essid += char(EEPROM.read(i));
  for (int i = 32; i < 64; ++i) // Reading Password
    Epass += char(EEPROM.read(i));
  for (int i = 64; i < 96; ++i) // Reading Password
    Etoken += char(EEPROM.read(i));
  if (Essid.length() == 0)
    Essid = "BLK";

  EtimeOpenDoor = EEPROM.read(200);
  EnumberEnterWrong = EEPROM.read(201);

  EtimeLock = EEPROM.read(202) * 100 + EEPROM.read(203);
  EenableChangePass = EEPROM.read(204);

  EtempThreshold1 = EEPROM.read(205);
  EtempThreshold2 = EEPROM.read(206);

  EhumiThreshold1 = EEPROM.read(207);
  EhumiThreshold2 = EEPROM.read(208);

  autoWarning = EEPROM.read(210);
  printValueSetup();
}

// ------------------------ Clear Eeprom ------------------------

void clearEeprom()
{
  Serial.println("Clearing Eeprom");
  for (int i = 0; i < 300; ++i)
    EEPROM.write(i, 0);
}

// -------------------- Hàm ghi data vào EEPROM ------------------
void writeEEPROM()
{
  clearEeprom();
  for (int i = 0; i < Essid.length(); ++i)
    EEPROM.write(i, Essid[i]);
  for (int i = 0; i < Epass.length(); ++i)
    EEPROM.write(32 + i, Epass[i]);
  for (int i = 0; i < Etoken.length(); ++i)
    EEPROM.write(64 + i, Etoken[i]);

  EEPROM.write(200, EtimeOpenDoor);     // lưu thời gian mở khóa
  EEPROM.write(201, EnumberEnterWrong); // lưu số lần nhập sai tối đa

  EEPROM.write(202, EtimeLock / 100);   // lưu hàng nghìn + trăm thời gian khóa
  EEPROM.write(203, EtimeLock % 100);   // lưu hàng chục + đơn vị thời gian khóa
  EEPROM.write(204, EenableChangePass); // lưu giá trị cho phép thay đổi mật khẩu

  EEPROM.write(205, EtempThreshold1); // lưu ngưỡng nhiệt độ 1
  EEPROM.write(206, EtempThreshold2); // lưu ngưỡng nhiệt độ 2

  EEPROM.write(207, EhumiThreshold1); // lưu ngưỡng độ ẩm 1
  EEPROM.write(208, EhumiThreshold2); // lưu ngưỡng độ ẩm 2
  EEPROM.commit();

  Serial.println("write eeprom");
  delay(500);
}

//-----------------------Task Task Button ----------
void TaskButton(void *pvParameters)
{
  while (1)
  {
    handle_button(&buttonOpen);
    handle_button(&buttonSET);
    handle_button(&buttonUP);
    handle_button(&buttonDOWN);
    delay(10);
  }
}
//-----------------Hàm xử lí nút nhấn nhả ----------------------
void button_press_short_callback(uint8_t button_id)
{
  switch (button_id)
  {
  case BUTTON1_ID:
    buzzerBeep(1);
    Serial.println("btSET press short");
    break;
  case BUTTON2_ID:
    buzzerBeep(1);
    Serial.println("btUP press short");
    break;
  case BUTTON3_ID:
    Serial.println("btDOWN press short");
    digitalWrite(RELAY, ENABLE);
    buzzerBeep(3);
    vTaskDelay(EtimeOpenDoor * 1000 / portTICK_PERIOD_MS);
    digitalWrite(RELAY, DISABLE);
    break;
  case BUTTON4_ID:
    Serial.println("OPERN DOOR");
    controlLock(DISABLE);
    buzzerBeep(2);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    controlLock(ENABLE);
    break;
  }
}
//-----------------Hàm xử lí nút nhấn giữ ----------------------
void button_press_long_callback(uint8_t button_id)
{
  switch (button_id)
  {
  case BUTTON1_ID:
    buzzerBeep(2);
    enableShow1 = DISABLE;
    Serial.println("btSET press long");
    screenOLED = SCREEN9;
    clearOLED();
    connectAPMode();
    break;
  case BUTTON2_ID:
    buzzerBeep(2);
    Serial.println("btUP press short");
    break;
  case BUTTON3_ID:
    buzzerBeep(2);
    Serial.println("btDOWN press short");
    OLED_STRING3 = "Canh bao";
    enableShow1 = DISABLE;
    autoWarning = 1 - autoWarning;
    EEPROM.write(210, autoWarning);
    EEPROM.commit();
    Blynk.virtualWrite(V1, autoWarning);
    if (autoWarning == 0)
      screenOLED = SCREEN11;
    else
      screenOLED = SCREEN10;
    break;
  case BUTTON4_ID:
    Serial.println("OPERN DOOR");
    controlLock(DISABLE);
    buzzerBeep(2);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    controlLock(ENABLE);
    break;
  }
}
// ---------------------- Hàm điều khiển còi -----------------------------
void buzzerBeep(int numberBeep)
{
  for (int i = 0; i < numberBeep; ++i)
  {
    digitalWrite(BUZZER, ENABLE);
    delay(100);
    digitalWrite(BUZZER, DISABLE);
    delay(100);
  }
}
void buzzerWarningBeep(int time)
{
  digitalWrite(BUZZER, ENABLE);
  delay(time);
  digitalWrite(BUZZER, DISABLE);
}
// ---------------------- Hàm điều khiển LED -----------------------------
void blinkLED(int numberBlink)
{
  for (int i = 0; i < numberBlink; ++i)
  {
    digitalWrite(LED, DISABLE);
    delay(300);
    digitalWrite(LED, ENABLE);
    delay(300);
  }
}

//-------------------------------- Kiểm tra nhiệt độ độ ẩm -------------------------//
/**
 * @brief Kiểm tra nhiệt độ độ ẩm và gửi lên BLYNK
 *
 * @param autoWarning auto Warning
 * @param temp Nhiệt độ hiện tại    *C
 * @param humi Độ ẩm hiện tại        %
 */

void check_air_quality_and_send_to_blynk(bool autoWarning, int temp, int humi)
{
  String notifications = "";
  int tempIndex = 0;
  int dustIndex = 0;
  int humiIndex = 0;
  if (dht11ReadOK == true)
  {
    if (autoWarning == 0)
    {
      if (temp < EtempThreshold1)
        tempIndex = 1;
      else if (temp >= EtempThreshold1 && temp <= EtempThreshold2)
        tempIndex = 2;
      else
        tempIndex = 3;

      if (humi < EhumiThreshold1)
        humiIndex = 1;
      else if (humi >= EhumiThreshold1 && humi <= EhumiThreshold2)
        humiIndex = 2;
      else
        humiIndex = 3;
      notifications = snTemp[tempIndex] + String(temp) + "*C . " + snHumi[humiIndex] + String(humi) + "% . ";

      Blynk.logEvent("check_data", notifications);
    }
    else
    {
      if (temp < EtempThreshold1)
        tempIndex = 1;
      else if (temp >= EtempThreshold1 && temp <= EtempThreshold2)
        tempIndex = 0;
      else
        tempIndex = 3;

      if (humi < EhumiThreshold1)
        humiIndex = 1;
      else if (humi >= EhumiThreshold1 && humi <= EhumiThreshold2)
        humiIndex = 0;
      else
        humiIndex = 3;
      if (tempIndex == 0 && humiIndex == 0 && dustIndex == 0)
        notifications = "";
      else
      {
        if (tempIndex != 0)
          notifications = notifications + snTemp[tempIndex] + String(temp) + "*C . ";
        if (humiIndex != 0)
          notifications = notifications + snHumi[humiIndex] + String(humi) + "% . ";
        Blynk.logEvent("auto_warning", notifications);
      }
    }

    Serial.println(notifications);
  }
}
