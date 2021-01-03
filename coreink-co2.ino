#include <WiFi.h>

#include <M5CoreInk.h>
#include <Adafruit_SGP30.h>
#include "coreink-co2-conf.h"

const char* ntpServer =  "ntp.jst.mfeed.ad.jp";
const long  gmtOffset_sec = 9 * 3600;
const int   daylightOffset_sec = 0;

uint16_t YY , MM , dd , hh, mm , ss;
int sensor_interval_time = 1; // 1sec
int sensor_interval_cnt = 30; // 30count
int co2_average = 0; 
int tvoc_average = 0; 

Adafruit_SGP30 sgp;

Ink_Sprite InkPageSprite(&M5.M5Ink);

void DrawCo2(int ppm)
{
  char ppm_str[8];
  sprintf(ppm_str, "%5d", ppm);
//  InkPageSprite.clear();
  InkPageSprite.drawString(10, 39, ppm_str, &AsciiFont24x48);
  InkPageSprite.drawString(130, 39, "ppm", &AsciiFont24x48);
  InkPageSprite.pushSprite();
}

void DrawTVOC(int ppm)
{
  char ppm_str[8];
  sprintf(ppm_str, "%5d", ppm);
//  InkPageSprite.clear();
  InkPageSprite.drawString(10, 85, ppm_str, &AsciiFont24x48);
  InkPageSprite.drawString(130, 85, "ppb", &AsciiFont24x48);
  InkPageSprite.pushSprite();
}

void DrawDate()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char date_str[20];
  YY = timeinfo.tm_year + 1900; MM = timeinfo.tm_mon + 1;
  dd = timeinfo.tm_mday;        hh = timeinfo.tm_hour;
  mm = timeinfo.tm_min;         ss = timeinfo.tm_sec;

  sprintf(date_str, "%0d/%0d/%0d %02d:%02d:%02d", YY, MM, dd, hh, mm, ss);

  InkPageSprite.drawString(20, 180, date_str);
  InkPageSprite.pushSprite();
}

void DrawBootMsg()
{
  InkPageSprite.drawString(20, 180, "booting and setup...");
  InkPageSprite.pushSprite();
}

void setup() {

  M5.begin();
  if ( !M5.M5Ink.isInit())
  {
    Serial.printf("Ink Init faild");
  }
  
  M5.M5Ink.clear();

  delay(1000);
  //creat ink refresh Sprite
  if ( InkPageSprite.creatSprite(0, 0, 200, 200, true) != 0 )
  {
    Serial.printf("Ink Sprite creat faild");
  }
  DrawBootMsg();
  
  Serial.begin(115200);
  Serial.println("SGP30 test");
  if (! sgp.begin()) {
    Serial.println("Sensor not found :(");
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf(".");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}

void loop() {

  co2_average=0;
  tvoc_average=0;
  for (int i = 0; i < sensor_interval_cnt; i++) {
    if (! sgp.IAQmeasure()) {
      Serial.println("Measurement failed");
      return;
    }
    co2_average += sgp.eCO2;
    tvoc_average += sgp.TVOC;
    delay(sensor_interval_time*1000);
  }
  
  co2_average = co2_average / sensor_interval_cnt;
  tvoc_average = tvoc_average / sensor_interval_cnt;

  DrawCo2(co2_average);
  DrawTVOC(tvoc_average);
  DrawDate();
  /*    if( M5.BtnUP.wasPressed()) ButtonTest("Btn UP Pressed");
      // if( M5.BtnDOWN.wasPressed()) ButtonTest("Btn DOWN Pressed");
      if( M5.BtnDOWN.wasPressed()){
      }
      if( M5.BtnMID.wasPressed()) ButtonTest("Btn MID Pressed");
      if( M5.BtnEXT.wasPressed()) ButtonTest("Btn EXT Pressed");
      if( M5.BtnPWR.wasPressed()){
          ButtonTest("Btn PWR Pressed");
          M5.PowerDown();
      }
  */
  M5.update();
}
