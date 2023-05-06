// wiring with UNO or Mega2560:
//--------------POWER Pins--------------------------------
//   5V  connects to DC 5V
//   GND connects to Ground
//   3V3 do not need to connect(NC)
//--------------LCD Display Pins--------------------------
//   LCD_RD   connects to Analog pin A0
//   LCD_WR   connects to Analog pin A1
//   LCD_RS   connects to Analog pin A2
//   LCD_CS   connects to Analog pin A3
//   LCD_RST  connects to Analog pin A4
//   LCD_D0   connects to digital pin 8
//   LCD_D1   connects to digital pin 9
//   LCD_D2   connects to digital pin 2
//   LCD_D3   connects to digital pin 3
//   LCD_D4   connects to digital pin 4
//   LCD_D5   connects to digital pin 5
//   LCD_D6   connects to digital pin 6
//   LCD_D7   connects to digital pin 7
//--------------SD-card fuction Pins ----------------------
//This Connection Only for UNO, Do not support Mega2560
//because they use different Hardware-SPI Pins
//SD_SS    connects to digital pin 10
//SD_DI    connects to digital pin 11
//SD_DO    connects to digital pin 12
//SD_SCK   connects to digital pin 13

#include <TimerOne.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <MsTimer2.h>
#include "DHT.h"

// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321

#define DHT_PIN A5 // Digital pin connected to the DHT sensor
#define FAN_PIN 10
#define RPM_PIN 11
#define SERVO_PIN 12
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define LCD_CS A3    // Chip Select goes to Analog 3
#define LCD_CD A2    // Command/Data goes to Analog 2
#define LCD_WR A1    // LCD Write goes to Analog 1
#define LCD_RD A0    // LCD Read goes to Analog 0

#define YP A3 // must be an analog pin, use "An" notation!
#define XM A2 // must be an analog pin, use "An" notation!
#define YM 9  // can be a digital pin
#define XP 8  // can be a digital pin

// Color definitions
#define ILI9341_BLACK 0x0000       /*   0,   0,   0 */
#define ILI9341_NAVY 0x000F        /*   0,   0, 128 */
#define ILI9341_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define ILI9341_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define ILI9341_MAROON 0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE 0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE 0x7BE0       /* 128, 128,   0 */
#define ILI9341_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define ILI9341_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define ILI9341_BLUE 0x001F        /*   0,   0, 255 */
#define ILI9341_GREEN 0x07E0       /*   0, 255,   0 */
#define ILI9341_CYAN 0x07FF        /*   0, 255, 255 */
#define ILI9341_RED 0xF800         /* 255,   0,   0 */
#define ILI9341_MAGENTA 0xF81F     /* 255,   0, 255 */
#define ILI9341_YELLOW 0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE 0xFFFF       /* 255, 255, 255 */
#define ILI9341_ORANGE 0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define ILI9341_PINK 0xF81F

//Param For 2.4"_9341_V3.0
#define TS_MINX 75
#define TS_MAXX 890

#define TS_MINY 110
#define TS_MAXY 920

#define MINPRESSURE 200
#define MAXPRESSURE 600

#define LCD_X 320
#define LCD_Y 240

#define RPM_DELTA_T 2000
#define DHT_DELTA_T 2333
#define SERVO_DELTA_T 23

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHT_PIN, DHTTYPE);

float temp = 12.3;
float RH = 12.3;
int rpm = 1000;
unsigned char rpm_sta = 0;
unsigned char rpm_gui_cnt = 0;
int rpm_cnter[5] = {0};
int pwm_10m = 512;
int my_x = 0;
int my_y = 0;
unsigned long dht_refresh_last = 0;
unsigned long rpm_refresh_last = 0;
unsigned long servo_refresh_last = 0;

void pwm_gui_refresh()
{
  tft.fillRect(0, 0, (long)LCD_X * pwm_10m / 1023, 60, ILI9341_GREENYELLOW);
  tft.fillRect((long)LCD_X * pwm_10m / 1023, 0, (long)LCD_X * (1023 - pwm_10m) / 1023 + 1, 60, ILI9341_DARKGREEN);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(14, 14);
  tft.print("PWM ");
  tft.print(pwm_10m * (long)100 / 1024, DEC);
  tft.print(" (%)");
}

void dht22_refresh()
{
  tft.fillRect(0, 60, LCD_X, 60, ILI9341_MAROON);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(14, 74);
  tft.print("TMP ");
  tft.setTextColor(ILI9341_RED);
  tft.print(temp, 1);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(" C");

  tft.fillRect(0, 120, LCD_X, 60, ILI9341_NAVY);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(14, 134);
  tft.print("R.H ");
  tft.setTextColor(ILI9341_CYAN);
  tft.print(RH, 1);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(" %");
}

void rpm_refresh()
{
  tft.fillRect(0, 180, LCD_X, 60, ILI9341_ORANGE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(14, 194);
  tft.print("RPM ");
  tft.setTextColor(ILI9341_YELLOW);
  tft.print(rpm);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(" R");
}

void pwm_cnter()
{
  rpm_sta <<= 1;
  if (digitalRead(RPM_PIN))
  {
    rpm_sta += 1;
  }
  if ((rpm_sta & 0x0f) == 0b1100)
  {
    rpm_cnter[0]++;
    rpm_cnter[1]++;
    rpm_cnter[2]++;
    rpm_cnter[3]++;
    rpm_cnter[4]++;
  }
}

void setup(void)
{
  // Serial.begin(115200);

  // 40 us = 25 kHz
  Timer1.initialize(40);
  Timer1.pwm(FAN_PIN, pwm_10m);

  tft.reset();
  uint16_t identifier = tft.readID();
  // if (identifier == 0x0101)
  // {
  //   identifier = 0x9341;
  // }
  // if (identifier == 0x9341)
  // {
  //   Serial.println(F("ILI9341 LCD detected"));
  // }
  tft.begin(identifier);
  tft.setRotation(1);

  // Serial.print(F("res:"));
  // Serial.print(tft.width());
  // Serial.print(F("x"));
  // Serial.println(tft.height());

  tft.fillScreen(ILI9341_LIGHTGREY);

  dht.begin();

  pinMode(RPM_PIN, INPUT_PULLUP);
  digitalWrite(SERVO_PIN, LOW);
  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);

  pwm_gui_refresh();
  dht22_refresh();
  rpm_refresh();

  // 2ms period
  MsTimer2::set(2, pwm_cnter);
  MsTimer2::start();
}

void loop(void)
{
#if 1
  TSPoint p = ts.getPoint();
  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    my_x = constrain(p.y, TS_MINX, TS_MAXX);
    my_x = map(my_x, TS_MINX, TS_MAXX, 0, LCD_X);

    my_y = constrain(p.x, TS_MINY, TS_MAXY);
    my_y = map(my_y, TS_MINY, TS_MAXY, LCD_Y, 0);

    if (my_y < 80)
    {
      //change pwm
      pwm_10m = map(constrain(p.y, TS_MINX, TS_MAXX), TS_MINX, TS_MAXX, 0, 1023);
      Timer1.pwm(FAN_PIN, pwm_10m);
      pwm_gui_refresh();
    }
    else if (my_y < 160)
    {
      if (my_x < LCD_X / 2)
      {
        pwm_10m = 0;
        Timer1.pwm(FAN_PIN, pwm_10m);
        pwm_gui_refresh();
      }
      else
      {
        pwm_10m = 1023;
        Timer1.pwm(FAN_PIN, pwm_10m);
        pwm_gui_refresh();
      }
    }
    else
    {
      if (millis() > servo_refresh_last + SERVO_DELTA_T)
      {
        servo_refresh_last = millis();
        digitalWrite(SERVO_PIN, HIGH);
        delayMicroseconds(map(constrain(p.y, TS_MINX, TS_MAXX), TS_MINX, TS_MAXX, 0, 1900));
        digitalWrite(SERVO_PIN, LOW);
      }
    }
  }
#endif

#if 1
  if (millis() > dht_refresh_last + DHT_DELTA_T)
  {
    dht_refresh_last = millis();
    temp = dht.readTemperature();
    RH = dht.readHumidity();
    dht22_refresh();
  }
#endif

#if 1
  if (millis() > rpm_refresh_last + RPM_DELTA_T)
  {
    rpm_refresh_last = millis();
    rpm = rpm_cnter[rpm_gui_cnt] * 3;
    rpm_cnter[rpm_gui_cnt] = 0;
    if (++rpm_gui_cnt == 5)
    {
      rpm_gui_cnt = 0;
    }

    rpm_refresh();
  }
#endif
}
