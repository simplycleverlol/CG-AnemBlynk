//--настройки, не используемое закомментировать
#define USE_BLYNK // использовать Blynk (работает только с есп32)
#define ConnectPortal // подключатьсz к WIFI (работает только с есп32), влияет на работу BLYNK
#define StartAP // поднимать точку доступа (работает только с есп32)
#define USE_WEBPAGE // поднимать веб страницу (работает только с есп32)

#define USE_LCD // использовать дисплей 1602
#define BLYNK_PRINT Serial
#define ESP8266



#ifdef ESP32
// подключаем библиотеку для WiFi-связи:
#include <WiFi.h>
#endif

#ifdef ESP8266
// подключаем библиотеку для WiFi-связи:
#include <ESP8266WiFi.h>
#endif

#if defined(ESP32) || defined(ESP8266)
#ifdef ConnectPortal
 #include <ESPAsyncWiFiManager.h>  
#endif

#ifdef USE_BLYNK
#include <BlynkSimpleStream.h>
char auth[] = "6KWyYFwvQL5GIXBw-Lp--aLdJ-W_TQNU"; //токен для блинка
WiFiClient blynkclient;
#endif

#ifdef StartAP
// здесь пишем данные для точки доступа:
const char* ssid     = "CG-ANEM";
const char* password = "climateguard";
DNSServer dns;
#endif

#ifdef USE_WEBPAGE
WiFiServer server(80);
String header;
#endif

#endif//объявления для есп32

#include <Wire.h>
#include <cgAnem.h> // библиотека анемометра

#ifdef USE_LCD
#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27,20,4);
#else
class Nolcd{ //заглушка вместо библиотеки дисплея (костыль)
  public:
  Nolcd(){};
  void init(){};
  void backlight(){};
  void setCursor(byte, byte){};
  void setContrast(byte){};
  void clear(){};
  void print(const char *){};
  void print(float){};
  void print(int){};
};
Nolcd lcd;
#endif

ClimateGuard_Anem cganem(ANEM_I2C_ADDR);
/*
      рассмотрим возможности библиотеки анемометра

  // здесь переменные, актуальные значения которых можно брать после вызова cganem.data_update()
  // это стоит использовать, если вам одновременно нужно несколько значений от анемометра (нампример, скорость воздуха и протекающий объем)

    float temperature //температура воздуха
    float airConsumption //объем проходящего воздуха (необходимо выставить сечение трубы/канала, в котором находится датчик)
    float airflowRate //поток водуха

    float ductArea //сечение трубы в см^2, можно изменить методом cganem.set_duct_area(float area) или просто cganem.ductArea = area;

    // методы, возвращающие актуальное значение без вызова cganem.data_update()
    
    float getTemperuture(); // температура воздуха
    float getAirflowRate(); // поток воздуха
    float calculateAirConsumption(); //объем проходящего воздуха
    
    //состояние датчика
    bool getSensorStatus(); //чтение регистра состояния (возвращает 1, если чтение успешно), после чего становятся актуальными значения:

      bool unsteadyProcess //между соседними измерениями разность температур холодного и горячего щупа слишком велика (>0.2 *C)
      bool overVcc //перенапряжение, модуль нельзя использовать
      bool taringError //ошибка калибровки

    //другие методы
    void set_duct_area(float area); // установить диаметр трубы
    uint8_t getChipId();
    uint8_t getFirmwareVersion();
    
    bool register_read_byte(uint8_t regAddr, uint8_t *retrieveData); // чтение байта регистра (для тех, кто прочитал даташит)
*/
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  if(!cganem.init()){//обязательная инициализация анемометра
  lcd.clear();
  lcd.print("ANEM ERROR!");
  Serial.println("ANEM ERROR!");
  delay(1000);
  #ifdef ESP32
  ESP.restart();
  #endif
  } 
  cganem.ductArea = 10.8*5.4; //здесь надо указать диаметр своей трубы

  lcd.clear();
  lcd.setContrast(255);
  lcd.print("CG-Anemometer");
  lcd.setCursor(0, 1);
  lcd.print("heating...");
  delay(2000);

 #if defined(ESP32) || defined(ESP8266)// актуально только для есп, не для ардуино
  // Создаем точку для подкоючения к вайфай
  #ifdef ConnectPortal
  AsyncWebServer serverdev(80);
  AsyncWiFiManager wifiManager(&serverdev,&dns);
  lcd.clear();
  lcd.print("Selecting wifi");
  lcd.setCursor(0, 1);
  lcd.print("open ConnectAnem");

  wifiManager.autoConnect("ConnectAnem");
  serverdev.end();
  #endif

  #ifdef USE_BLYNK
  blynkclient.connect(BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
  Blynk.begin(blynkclient, auth); //инит блинка
  #endif

  #ifdef StartAP //подъем точки доступа
  WiFi.softAP(ssid, password);
  #endif

  #ifdef USE_WEBPAGE //сервер для веб странички
  server.begin();
  #endif
  #ifdef StartAP
  lcd.clear();
  lcd.print("SSID: ");
  lcd.print(ssid);
  lcd.setCursor(0, 1);
  lcd.print("pwd:");
  lcd.print(password);
  delay(2000);
  lcd.clear();
  lcd.print("AP web IP: ");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.softAPIP().toString().c_str());
  #endif
  delay(1000);
  #ifdef ConnectPortal
  lcd.clear();
  lcd.print("Extern web IP: ");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString().c_str());
  #endif
  #endif
  delay(4000);

  Serial.println("Windspeed, temp, flow"); //заголовки для плоттера

}
  //#include <webpage.h>
void loop(){
    static unsigned long timer = millis();
    
   if(millis() - timer > 1000){
    timer = millis();
    cganem.getSensorStatus();
    cganem.data_update();
    lcd.setCursor(0, 0);
    lcd.print("Air spd: ");
    lcd.print(cganem.airflowRate);
    lcd.print("     ");
    lcd.setCursor(0, 1);
    lcd.print("Temp:    ");
    lcd.print(cganem.temperature); //выводим данные на экран
    lcd.print("     ");
    #if (defined(ESP32) || defined(ESP8266)) && defined(USE_BLYNK)
    Blynk.virtualWrite(V0, cganem.airflowRate);
    Blynk.virtualWrite(V1, cganem.temperature);
    Blynk.virtualWrite(V2, cganem.airConsumption); //на сервер блинк
    Blynk.virtualWrite(V3, cganem.unsteadyProcess*255); //на сервер блинк
    Blynk.virtualWrite(V4, cganem.taringError*255); //на сервер блинк

    Blynk.run();
    #endif
 
    Serial.print(cganem.airflowRate);
    Serial.print(" ");
    Serial.print(cganem.temperature);
    Serial.print(" ");
    Serial.println(cganem.airConsumption);//в ком порт (можно открыть плоттер)
  }
  #ifdef USE_WEBPAGE
  printPage(); //веб страничка (в другом файле описана)
  #endif
  
}