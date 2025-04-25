/*********************************************************************
Программа для проверки OLED дисплея 128x32 с интерфейсом I2C
Вывод 4 используется для сброса дисплея
Измерение температуры производится с помощью датчика LM75A
Индикация со статическим отображением полей, переключение полей 
производится с помощью кнопки S1 на плате, (подключена к выводу PD4) 
Корректная работа в области отрицательных температур
*********************************************************************/
//--------------------------Подключаемые файлы-----------------------
#include "cMeteo.h"
#include "cKey.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_BMP280.h"  // Добавляем драйвер для BMP280
#include <iarduino_DHT.h>     // подключаем библиотеку для работы с датчиком DHT
//------------------------------Константы----------------------------
#define OLED_RESET 4
#define DHT_PIN 2                 // Вывод, к которому подключают датчик DHT

#define DHT_DWELL 2000            //Период опроса датчика
#define SHOW_DWELL 200            //Период обновления индикации
#define MAX_FONT_SIZE 3           // Максимальный размер шрифта, доступный на экране 128х32
#define CURSOR_X 10                // Начальный сдвиг курсора по X
#define CURSOR_Y 10                // Начальный сдвиг курсора по Y
#define CURSOR_JITTER 3           // Максимальное значение дрожания курсора
#define S1 4                      // Вывод для подключения кнопки S1
#define DISPLAY_PAGE_EDGE 2       // Число режимов индикации

//-------------------------Глобальные переменные---------------------

iarduino_DHT sensor(DHT_PIN);     
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BMP280 bmp;  

cKey KeyS;
long cKey::CurrentTime;
int cKey::Dwell = DELAY_KEYBOARD;

long MillisCurrent;               // реализация задержки
long MillisPreviousMesurement;    //
long MillisPreviousShow;          //

float fPressurePa;
float fAltitude;
float fPressureMm;
float fBMP280Temperature;

float fLM75Temperature;

int DHTStatus;                  // Статус датчика влажности

int DisplayPage = 0;            // Номер страницы, отображаемой на дисплее
                                // изменяется при нажатии на кнопку S1


String sBufferT = "";
String sBufferP = "";
String sBufferH = "";

//Смещение на экране
int RandomX;
int RandomY;

String sVersion = "Meteo v2.5.1";
String sBuffer = sVersion;

//---------------------Программа начальной установки-----------------
void setup()
{                
  Serial.begin(115200);
  Serial.println(sVersion);

  KeyS.install('S', S1);
 
  if (!bmp.begin()) 
    Serial.println("BMP280 err");
  else
    Serial.println("BMP280 Ok");

  //Инициализация дисплея, начальная заставка
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.display();
  display.clearDisplay();

  MillisCurrent = millis();       // настройка задержки
  MillisPreviousMesurement = MillisCurrent;
  MillisPreviousShow = MillisCurrent;

  DHTStatus = DHT_OK;
  
}//End of void setup()

//------------------------Главный цикл программы---------------------
void loop() 
{
  MillisCurrent = millis();
  //---------------------- Реализация клавиатуры --------------------
  KeyS.operate();
  
  if(KeyS.checkEvent())
  {
    DisplayPage++;
    if(DisplayPage > DISPLAY_PAGE_EDGE) DisplayPage = 0; 
  }
  
  //----------------- Индикация результата на дисплее ---------------
  if(MillisCurrent - MillisPreviousShow > SHOW_DWELL)
  {
    MillisPreviousShow = MillisCurrent;
    //---
    showPage();
    //---
 
  }//End of if(MillisCurrent - MillisPreviousShow > SHOW_DWELL)
    
  //------------------- Измерение физических величин -----------------
  if(MillisCurrent - MillisPreviousMesurement > DHT_DWELL)
  {
    MillisPreviousMesurement = MillisCurrent;
    
    //---
      //Измерение давления с помощью датчика BMP280T
      fPressurePa = bmp.readPressure();
      fAltitude = bmp.readAltitude(ALTITUDE_LEVEL);
      fBMP280Temperature = bmp.readTemperature();
      fPressureMm = fPressurePa / 133.3;

      //Измерение температуры с помощью датчика LM75A
      Wire.requestFrom(0x4f,2); //запросить 1 байта данных
      uint8_t high = Wire.read();
      uint8_t low = Wire.read();
      
      uint16_t value = ((((uint16_t)high) << 8) | (uint16_t)low);
      if((high & 0x8000) > 0) value = (!value) + 1;
      value = value >> 5;
      fLM75Temperature = (float)value / 8; 

      if(fLM75Temperature > 60.0)
      {
        fLM75Temperature = fLM75Temperature - 255.0; 
      }

      // Измерение температуры и влажности с помощью датчика DHT-11
      DHTStatus = sensor.read();

      // Представление данных
      sBufferT = "T=";
      sBufferT += (String)(int)(fLM75Temperature);
      //sBufferT += (String)(int)(fBMP280Temperature);
      sBufferT += "*C";
     
      sBufferP = "P=";
      sBufferP += (String)(int)fPressureMm;
      //sBufferP += "mm";

      switch(DHTStatus)
      {    
        // читаем показания датчика
        case DHT_OK:               sBufferH =("Hum=" + (String)(int)sensor.hum /*+ "%"*/);  break;
        case DHT_ERROR_CHECKSUM:   sBufferH =("HErr=1");                               break;
        case DHT_ERROR_DATA:       sBufferH =("HErr=2");                               break;
        case DHT_ERROR_NO_REPLY:   sBufferH =("HErr=3");                               break;
        default:                   sBufferH =("HErr=4");                               break;
      }
       //prepareDataForShow();//Подготовка данных для индикации
    
  }//End of if(MillisCurrent - MillisPreviousMesurement > DHT_DWELL)

}//End of void loop()

//-----------------------------Подпрограммы--------------------------

void prepareDataForShow()
{
  switch(DisplayPage)
  {
    case 0:
    sBuffer = "T=";
    sBuffer += (String)(int)(fLM75Temperature);
    //sBuffer += (String)(int)(fBMP280Temperature);
    sBuffer += "*C";
    break;

    case 1:
    sBuffer = "P=";
    sBuffer += (String)(int)fPressureMm;
    //sBuffer += "mm";
    break;

    case 2:
    switch(DHTStatus)
    {    
      // читаем показания датчика
      case DHT_OK:               sBuffer =("H=" + (String)(int)sensor.hum + " %");  break;
      case DHT_ERROR_CHECKSUM:   sBuffer =("HErr=1");                               break;
      case DHT_ERROR_DATA:       sBuffer =("HErr=2");                               break;
      case DHT_ERROR_NO_REPLY:   sBuffer =("HErr=3");                               break;
      default:                   sBuffer =("HErr=4");                               break;
    }
    break;

    default:
    break;
  }//End of switch(DisplayPage)
   
}//End of void prepareDataForShow()

void showPage()
{
    String sViewBuffer = "======";
    if(KeyS.getValue() > 0)
    {
      RandomX = 0;
      RandomY = 0;
    }
    else
    {
      RandomX = random(CURSOR_JITTER);
      RandomY = random(CURSOR_JITTER);

      prepareDataForShow();//Подготовка данных для индикации
      
      sViewBuffer = sBuffer;
    }
    int TextSize = 2;
    display.clearDisplay();
    display.setTextSize(TextSize);
    display.setTextColor(WHITE);
    display.setCursor(CURSOR_X + RandomX, CURSOR_Y + RandomY);
    //display.setCursor(10, 10);
    display.println(sViewBuffer);
    display.display();
}
