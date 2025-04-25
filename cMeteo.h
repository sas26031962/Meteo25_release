#ifndef C_METEO_h
#define C_METEO_h
/*=========================================================================
 ПОДКЛЮЧАЕМЫЕ ФАЙЛЫ
 ==========================================================================*/
#include "Adafruit_BMP280.h"  // Добавляем драйвер для BMP280

//=========================================================================  
// КОНСТАНТЫ
//=========================================================================  
#define ALTITUDE_LEVEL 1013.25    //Уровень расчёта высоты
#define TIMEOUT 2000              // Задержка между измерениями (мс)


/*=========================================================================
 КЛАССЫ
 ==========================================================================*/
class cMeteo{
//=========================================================================  
// АТРИБУТЫ
//=========================================================================  
int error;

Adafruit_BMP280 bmp;     // 
//dht11 dht;             // 

float fTemperature;
float fPressurePa;
float fAltitude;
float fPressureMm;

float fDTH11Temperature;
float fHumidity;
float fPT1000Temperature;

double PreviousMillis;
double CurrentMillis;

public:
//Adafruit_SSD1306 display(OLED_RESET);
//Adafruit_SSD1306 display(4);

String sStatus;
String sBMP280Temperature;
String sBMP280Pressure;
String sBMP280Altitude;
String sDTH11Humidity;
String sDTH11Temperature;
String sPT1000Temperature;

//=========================================================================  
// МЕТОДЫ
//=========================================================================  
protected:

void getHumidity();
void parseError();
void getData();
void showData();
void serialData();

public:

void operate();
void install();

//=========================================================================  
// КОНСТРУКТОРЫ
//=========================================================================  
cMeteo();
  
};//End of class cMeteo

cMeteo::cMeteo()
{
  this->error = 0;
  this->fTemperature = 1000.0;
  this->fPressurePa = 10;
  this->fAltitude = -1000.0;
  this->fPressureMm = -1000.0;

  this->fPT1000Temperature = -1000.0;
  
  this->fDTH11Temperature = 1000.0;
  this->fHumidity = 1000.0;

  this->CurrentMillis = millis();
  this->PreviousMillis = this->CurrentMillis;

  this->fPressureMm = -1000.0;

  this->sStatus = "Meteo: Ok!";
  
}//End of cMeteo::cMeteo()

void cMeteo::getHumidity()
{
  //this->error = this->dht.read(DHT11_PIN);
}

void cMeteo::parseError()
{
  /*
  switch (this->error)
  {
    case DHTLIB_OK:  
      //this->sStatus = "Meteo: DHT11 Ok!";
    break;
    case DHTLIB_ERROR_CHECKSUM:
      this->sStatus = "Meteo: DHT11 Checksum error";
    break;
    case DHTLIB_ERROR_TIMEOUT:
      this->sStatus = "Meteo: DHT11 Timeout error";
    break;
    default:
      this->sStatus = "Meteo: DHT11 Unknown error";
    break;
  }
  */
}//End of void cMeteo::parseError()

void cMeteo::install()
{
  if (!this->bmp.begin()) 
  {  
    this->sStatus = "Meteo: BMP280 wiring error";
  }
}//End of cMeteo::install()

void cMeteo::getData()
{

    this->fTemperature = this->bmp.readTemperature();
    this->fPressurePa = this->bmp.readPressure();
    this->fAltitude = this->bmp.readAltitude(ALTITUDE_LEVEL);
    this->fPressureMm = this->fPressurePa / 133.3;

    //this->fDTH11Temperature = this->dht.temperature;
    //this->fHumidity = this->dht.humidity;
}

void cMeteo::showData()
{
  this->sBMP280Temperature = "BMP280 temperature: ";
  this->sBMP280Temperature += this->fTemperature;
  this->sBMP280Temperature += " *C";

  this->sBMP280Pressure = "BMP280 pressure: ";
  this->sBMP280Pressure += this->fPressurePa;
  this->sBMP280Pressure += " Pa (";
  this->sBMP280Pressure += this->fPressureMm;
  this->sBMP280Pressure += ") mm";

  this->sBMP280Altitude = "BMP280 approx altitude: ";
  this->sBMP280Altitude += fAltitude;
  this->sBMP280Altitude += " m over ";
  this->sBMP280Altitude += ALTITUDE_LEVEL;

  this->sDTH11Humidity = "DTH11Humidity: ";
  this->sDTH11Humidity += this->fHumidity;
  this->sDTH11Humidity += " %";

  this->sDTH11Temperature = "DTH11Temperature: ";
  this->sDTH11Temperature += this->fTemperature;
  this->sDTH11Temperature += " *C";
}

void cMeteo::serialData()
{
    Serial.println(this->sStatus);
    Serial.println(this->sBMP280Temperature);
    Serial.println(this->sBMP280Pressure);
    Serial.println(this->sBMP280Altitude);
    Serial.println(this->sDTH11Humidity);
    Serial.println(this->sDTH11Temperature);
    Serial.println();
}

void cMeteo::operate()
{
  this->CurrentMillis = millis();
  if(this->CurrentMillis - this->PreviousMillis > TIMEOUT)
  {
    this->PreviousMillis = this->CurrentMillis;
    
    this->getHumidity();
    this->parseError();
    this->getData();
    this->showData();
    this->serialData();
  }
}


#endif
