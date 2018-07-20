#include<SdFat.h>
#include<TouchScreen.h>
#include "Adafruit_MAX31856.h"
// You can use any 5 pins; see note below about hardware vs software SPI

#define TFT_DC P1S5
#define TFT_CS DAC


#define DRDY D3
#define MEASURE D4
#define POWERON D5
#define BTCON D7
#define BTAT D6
#define WAKEUP WKP
#define TEMPCS A0
#define FAULT A2
#define OPEN P1S4
#define BACKLIGHT D2
#define HOLD A1
#define LCD_YM P1S3
#define LCD_YP P1S2
#define LCD_XM P1S1
#define LCD_XP P1S0
#define YP P1S2  // must be an analog pin, use "An" notation!
#define XM P1S1  // must be an analog pin, use "An" notation!
#define YM P1S3   // can be a digital pin
#define XP P1S0   // can be a digital pin

#define TS_MINX 320
#define TS_MINY 3575
#define TS_MAXX 3580
#define TS_MAXY 450

#define MINPRESSURE 10
#define MAXPRESSURE 3000
// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF
#define ORANGE          0xFB20
#define GRAY            0x8410
#define DARKBROWN       0x9448
#define FAINTGRAY       0xA534
#define DARKGREEN       0x0620

#define LOW             0
#define HIGH            1
////////////////////////// EEPROM Address Definitions ///////////////////////
#define AT      1
#define NORMAL  0


#define SPI_CONFIGURATION 0

SdFat sd;
const uint8_t chipSelect = D2;
File myFile;


#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))




// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, 0);
TouchScreen ts = TouchScreen(XP, YP, XM, YM,300);
Adafruit_MAX31856 tc = Adafruit_MAX31856(TEMPCS);
// Create IntervalTimer objects
IntervalTimer myTimer;


////////////////       Float Variables      ////////////////////////////////

float p = 3.1415926;

////////////////      Pin Declarations     ////////////////////////////////

////////////////    Volatile Boolean Flags ////////////////////////////////

volatile boolean menuState = LOW;
volatile boolean menuFlagL = LOW;
volatile boolean menuFlagH = LOW;
volatile boolean sp1FlagH = LOW;
volatile boolean sp1FlagL = LOW;
volatile boolean nextinc010 = LOW;


//////////////      Volatile Temporary Variables   //////////////////////////

volatile int inc0,inc1,inc2,inc3,inc4,pos;
uint16_t  tempVar[10];
unsigned int seconds,prevSeconds;
uint8_t inc,next,tempNext;
uint8_t passInc = 0,passNext = 0,tempPass[4];
uint8_t password;

//////////////  Variables to be stored in EEPROM Memory  ///////////////////

uint8_t pvUnit,mode = 0;
uint8_t dataLogStatus = 0,dispScroll  = 1;
uint8_t  scanTime,scrollTime = 10;
boolean  heartFlag = 0;


uint8_t DEBUG_LIVE=0;
uint8_t DEBUG_BLUETOOTH=0;

///////////////// Read Only Variables //////////////////////////////////////
char serialNo[10],batchNo[4],sensorType,outputType;

//////////////////////   Other Variables    /////////////////////////////////
uint16_t color,fgColor,bgColor,selColor,clockColor,temperatureColor;
uint8_t tenthous,thous,hunds,ones,tens;
uint8_t i,j,k;
uint8_t dispChange = 0,displayMode = 0;
char charPos1[8] = {0,15,45,60,90,105};
char charPos2[8] = {0,15,30,45,60,75,90,105};
char charPos3[8] = {20,35,50,80};
uint8_t colon = 0;
uint8_t dispNow = 0;
uint16_t sensorValue = 0;  // variable to store the value coming from the sensor
float printValue;
volatile unsigned int setRelay,setDelay;
long  adcValue;
float sensorInput,displayValue;
long calDisp[5],calAdc[5];
long rangeLow,rangeHigh,zero,span;
double vin;
float f1;
int outputValue,turnDownRatio;
uint8_t setScreen = 0;
String WifiSSID,WifiPASS;
uint8_t wifiStatus = 0;
uint8_t rtcSec,rtcPrevSec;
int count = 0;
uint16_t colorValues[4] = {0x07E0,0xFFE0,0xFB20, 0xF800}; //GREEN,YELLOW,ORANGE,RED
//////////////////////  String Declarations     /////////////////////////////////
