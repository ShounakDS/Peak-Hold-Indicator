/*
 * Project Digital Pressure Controller Cum Tranmitter
 * Description:
 *
 * Author: Shounak Sharangpani
 * Date:
 */
#include <SparkIntervalTimer.h>
#include "Adafruit_mfGFX.h"
#include "Adafruit_ILI9341.h"
#include "Particle.h"
#include "application.h"
#include "fonts.h"
#include "Variables.h"
#include "SdFat.h"
#include "TouchScreen.h"
#include "Adafruit_MAX31856.h"
#include "Icons.h"

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
//STARTUP(WiFi.selectAntenna(ANT_INTERNAL)); // selects the u.FL antenna
//SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_MODE(AUTOMATIC);

TCPServer server = TCPServer(23);
TCPClient client;

unsigned int brightness = 10,holdTime = 0,wkupTime = 0;
float cjTemp;
int temperature,prevTemp,heatNo = 112;
boolean sleepFlag = LOW,holdTimeFlag = LOW,measureFlag = LOW,openFlag = LOW;
char addr[16];
/// Set up Function
void setup() {
  ////////////////////////////////    Pin Declarations   /////////////////////////////////////////////////////
  pinMode(POWERON, OUTPUT);
  pinMode(DRDY, INPUT_PULLDOWN);
  pinMode(FAULT, INPUT_PULLDOWN);
  pinMode(HOLD, OUTPUT);
  pinMode(OPEN, OUTPUT);
  pinMode(MEASURE, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);
  pinMode(WKP, INPUT_PULLDOWN);
  digitalWrite(POWERON,HIGH);
  digitalWrite(HOLD,HIGH);
  digitalWrite(OPEN,HIGH);
  digitalWrite(MEASURE,HIGH);
  //digitalWrite(BACKLIGHT,LOW);

  sleepFlag = LOW;
  //WiFi.on();
  //Particle.connect();
  seconds = 0;
  sleepFlag = 0;
  Serial.begin(115200);
  Time.zone(+5.5);
  IPAddress myAddress(192,168,1,150);
  IPAddress netmask(255,255,255,0);
  IPAddress gateway(192,168,1,1);
  IPAddress dns(192,168,1,1);
  WiFi.setStaticIP(myAddress, netmask, gateway, dns);
  // now let's use the configured IP
  WiFi.useStaticIP();
 ////////////////////////////////    Int Declarations   /////////////////////////////////////////////////////
 // Put initialization like pinMode and begin functions here.

/////////////////////////////   Initializing Peripherals  //////////////////////////////////////////////////
  myTimer.begin(timerISR, 2000, hmSec);
  tft.begin();
  tft.fillScreen(BLACK);
  tft.setCursor(0,0);
  tft.setTextWrap(LOW);
  tft.setRotation(1);
  digitalWrite(BACKLIGHT,HIGH);
  delay(100);

  /////////////////////////////   Initializing Variables  ////////////////////////////////////////
  color = GREEN;
  fgColor = WHITE;
  bgColor = BLACK;
  selColor = YELLOW;
  clockColor = YELLOW;
  temperatureColor = WHITE;
  /////////////////////////////   Serial Debug Initializing  //////////////////////////////////////
  // Define the pins used to communicate with the MAX31856

  Particle.variable("Seconds", &seconds);
 /////////////////////////////   EEPROM Address Read     ////////////////////////////////////////
  tc.begin();
  tc.setThermocoupleType(MAX31856_TCTYPE_R);
  tc.oneShotTemperature();
  /////////////////////////////   Wifi Status    ////////////////////////////////////////////////
  IPAddress localIP = WiFi.localIP();
  sprintf(addr, "%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
  Particle.publish("Address", addr);
  server.begin();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loop() runs over and over again, as quickly as it can execute.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop(){
  tft.fillScreen(BLACK);
  //passwordScreen();
  homeScreen();
  while(menuState == LOW){
    debugEvent();
    Particle.process();
    detectTouch();
    client = server.available();
    if (client.available()){
        while(client.connected()){
            char incoming = client.read();
            if (incoming == 'D'){
              String string1;
              string1 = String(heatNo) +"," + String(temperature) +","+String(measureFlag) +","+String(holdTimeFlag) +","+String(openFlag)+",";
              client.print(string1);
              client.stop();
            }
        }
      }
    if(seconds > prevSeconds){
      temperature = (int)measureTemp();
      if(holdTimeFlag){
        temperatureColor = RED;
        digitalWrite(HOLD,HIGH);
      }
      else{
        temperatureColor = WHITE;
        digitalWrite(HOLD,LOW);
      }
      tft.drawCircle(255,50,3,temperatureColor);
      tft.drawCircle(255,50,4,temperatureColor);
      tft.drawCircle(255,50,5,temperatureColor);
      tft.setFont(ARIAL_8);
      tft.drawChar(260,60,'C',temperatureColor,bgColor,3);///Print for BAR
      tft.setFont(ARIAL_36);
      if(temperature >= 1000){
         digitalWrite(MEASURE,HIGH);
         measureFlag = HIGH;
         hex2bcd(temperature);
         tft.drawChar(10,50,thous,temperatureColor,bgColor,2);
         tft.drawChar(70,50,hunds,temperatureColor,bgColor,2);
         tft.drawChar(130,50,tens,temperatureColor,bgColor,2);
         tft.drawChar(190,50,ones,temperatureColor,bgColor,2);///Print for BAR

         if(temperature == prevTemp){
           holdTime++;
         }
         else{
           holdTime = 0;
         }
         if(holdTime > 10){
           holdTimeFlag = 1;
         }
         else{
           holdTimeFlag = 0;
         }
      }
      else{
        digitalWrite(MEASURE,LOW);
        measureFlag = LOW;
        digitalWrite(HOLD,LOW);
        holdTime = 0;
        holdTimeFlag = LOW;
        tft.drawChar(10,50,'-',temperatureColor,bgColor,2);
        tft.drawChar(70,50,'-',temperatureColor,bgColor,2);
        tft.drawChar(130,50,'-',temperatureColor,bgColor,2);
        tft.drawChar(190,50,'-',temperatureColor,bgColor,2);///Print for BAR
        temperatureColor = WHITE;
      }
      powerOff();
      drawTime();
      //Particle.publish("Test","Test");
      prevTemp = temperature;
      prevSeconds = seconds;
    }
  }
  tft.fillScreen(BLACK);
  menuScreen();
  while(menuState == HIGH){
    // while in menu screen
    while(next == 0){
      Particle.process();
      detectTouch();
      if(seconds > prevSeconds){
        powerOff();
        prevSeconds = seconds;
      }
    }
    tft.fillScreen(BLACK);

    /////////////////////////////////////////////////////////////////////////
    // Time Screen
    /////////////////////////////////////////////////////////////////////////
    while(next == 1){
      timeScreen();
      while(1){
        Particle.process();
        if(next != 1){
          break;
        }
        detectTouch();
        if(seconds > prevSeconds){
          powerOff();
          prevSeconds = seconds;
        }
      }

    }
    /////////////////////////////////////////////////////////////////////////
    //Date  Screen
    /////////////////////////////////////////////////////////////////////////
    while(next == 2){
      dateScreen();
      while(1){
        Particle.process();
        if(next != 2){
          break;
        }
        detectTouch();
        if(seconds > prevSeconds){
          powerOff();
          prevSeconds = seconds;
        }
      }

    }
    // while in touched settings Icon
    /////////////////////////////////////////////////////////////////////////
    // password Screen
    /////////////////////////////////////////////////////////////////////////
    tft.fillScreen(BLACK);
    while(next == 8){
      passwordScreen();
      while(1){
        Particle.process();
        /////////////////////////////////////////////////////////////////////////
        // code to detect touch
        /////////////////////////////////////////////////////////////////////////
        detectTouchPassword();
        //////////////// end touch screen code //////////////////////////////////
        if(seconds > prevSeconds){
          powerOff();
          if(passNext > 3){
            int num = (tempPass[0]*1000) + (tempPass[1]*100) + (tempPass[2]*10) + tempPass[3];
            if(num == 1234){
              next = 5;
              break;
            }
            else{
              tft.setFont(ARIAL_12);
              tft.setCursor(0,110);
              tft.setTextSize(1);
              tft.setTextColor(WHITE,BLACK);
              tft.print("INCORRECT PASSWORD PLEASE TRY AGAIN");
              tft.setFont(ARIAL_8_N);
              tft.drawChar(40,50,'-',WHITE,BLACK,3);
              tft.drawChar(100,50,'-',WHITE,BLACK,3);
              tft.drawChar(160,50,'-',WHITE,BLACK,3);
              tft.drawChar(220,50,'-',WHITE,BLACK,3);///Print for BAR
              passNext = 0;
              next = 5;
            }

          }
          prevSeconds = seconds;
        }
      }
    }
    // while in touched settings Icon
    while(next == 4){
      Sleep();
    }
    /////////////////////////////////////////////////////////////////////////
    // Settings Screen
    /////////////////////////////////////////////////////////////////////////
    tft.fillScreen(BLACK);
    while(next == 3){
      settingScreen();
      while(1){
        Particle.process();
        /////////////////////////////////////////////////////////////////////////
        // code to detect touch
        /////////////////////////////////////////////////////////////////////////
        detectTouch();
        //////////////// end touch screen code //////////////////////////////////
        if(seconds > prevSeconds){
          powerOff();
          prevSeconds = seconds;
        }
      }
    }

  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Home screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void homeScreen(){

  tft.drawBitmap(0, 190, wifiIcon, 48, 48, MAGENTA);
  tft.drawBitmap(70, 190, bluetoothIcon, 48, 48, BLUE);
  tft.drawBitmap(140, 190, cloudIcon, 48, 48, WHITE);
  tft.drawBitmap(270, 190, menuIcon, 48, 48, WHITE);
  //tft.drawBitmap(100, 190, unitIcon, 48, 48, WHITE);
  tft.drawFastHLine(0, 18, 320, WHITE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Menu Screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void menuScreen(){
  tft.setFont(ARIAL_12);
  tft.setCursor(120,2);
  tft.setTextSize(1);
  tft.fillRect(0,0,320,20,YELLOW);
  tft.setTextColor(BLACK,YELLOW);
  tft.setCursor(110,2);
  tft.setTextSize(1);
  tft.print("M E N U");
  tft.drawBitmap(10, 40, clockIcon, 48, 48, WHITE);
  tft.drawBitmap(90,40, calendarIcon, 48, 48, WHITE);
  tft.drawBitmap(170, 40, settingsIcon, 48, 48, WHITE);
  tft.drawBitmap(250, 40, powerIcon, 48,48, WHITE);
  tft.drawBitmap(270, 190, backIcon, 48, 48, WHITE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Time Screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void timeScreen(){
  tft.setFont(ARIAL_12);
  tft.setCursor(120,2);
  tft.setTextSize(1);
  tft.fillRect(0,0,320,20,YELLOW);
  tft.setTextColor(BLACK,YELLOW);
  tft.setCursor(60,2);
  tft.setTextSize(1);
  tft.print("S E T  T H E   T I M E");
  tft.fillRoundRect(35,25,50,50,4,WHITE);
  tft.fillRoundRect(38,28,44,44,4,BLUE);
  tft.fillRoundRect(135,25,50,50,4,WHITE);
  tft.fillRoundRect(138,28,44,44,4,BLUE);
  tft.fillRoundRect(35,140,50,50,4,WHITE);
  tft.fillRoundRect(38,143,44,44,4,BLUE);
  tft.fillRoundRect(135,140,50,50,4,WHITE);
  tft.fillRoundRect(138,143,44,44,4,BLUE);
  tft.setTextColor(WHITE,BLUE);
  tft.setFont(ARIAL_36);
  tft.setCursor(48,30);
  tft.print("+");
  tft.setCursor(148,30);
  tft.print("+");
  tft.setCursor(48,145);
  tft.print("-");
  tft.setCursor(148,145);
  tft.print("-");
  tft.drawRoundRect(0,200,50,40,4,WHITE);
  tft.drawRoundRect(260,200,50,40,4,WHITE);
  tft.drawBitmap(5, 200, okIcon, 40, 40, WHITE);
  tft.drawBitmap(270, 200, clearIcon, 40, 40, WHITE);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Date Screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void dateScreen(){
  tft.setFont(ARIAL_12);
  tft.setCursor(120,2);
  tft.setTextSize(1);
  tft.fillRect(0,0,320,20,YELLOW);
  tft.setTextColor(BLACK,YELLOW);
  tft.setCursor(60,2);
  tft.setTextSize(1);
  tft.print("S E T  T H E   DATE");
  tft.fillRoundRect(35,25,50,50,4,WHITE);
  tft.fillRoundRect(38,28,44,44,4,BLUE);
  tft.fillRoundRect(135,25,50,50,4,WHITE);
  tft.fillRoundRect(138,28,44,44,4,BLUE);
  tft.fillRoundRect(35,140,50,50,4,WHITE);
  tft.fillRoundRect(38,143,44,44,4,BLUE);
  tft.fillRoundRect(135,140,50,50,4,WHITE);
  tft.fillRoundRect(138,143,44,44,4,BLUE);
  tft.setTextColor(WHITE,BLUE);
  tft.setFont(ARIAL_36);
  tft.setCursor(48,30);
  tft.print("+");
  tft.setCursor(148,30);
  tft.print("+");
  tft.setCursor(48,145);
  tft.print("-");
  tft.setCursor(148,145);
  tft.print("-");
  tft.drawRoundRect(0,200,50,40,4,WHITE);
  tft.drawRoundRect(260,200,50,40,4,WHITE);
  tft.drawBitmap(5, 200, okIcon, 40, 40, WHITE);
  tft.drawBitmap(270, 200, clearIcon, 40, 40, WHITE);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Settings Screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void settingScreen(){
  tft.setFont(ARIAL_12);
  tft.setCursor(120,2);
  tft.setTextSize(1);
  tft.fillRect(0,0,320,20,YELLOW);
  tft.setTextColor(BLACK,YELLOW);
  tft.setCursor(90,2);
  tft.setTextSize(1);
  tft.print("S E T T I N G S");
  tft.drawBitmap(10, 40, holdtimeIcon, 48, 48, WHITE);//drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,int16_t w, int16_t h, uint16_t color),
  tft.drawBitmap(90, 40, tcIcon, 48, 48, WHITE);
  tft.drawBitmap(170, 40, unitIcon, 48, 48, WHITE);
  tft.drawBitmap(250, 40, wifiIcon, 48,48, WHITE);
  tft.drawBitmap(270, 190, backIcon, 48, 48, WHITE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  password Screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void passwordScreen(){
  tft.setFont(ARIAL_8_N);
  tft.drawChar(40,50,'-',WHITE,BLACK,3);
  tft.drawChar(100,50,'-',WHITE,BLACK,3);
  tft.drawChar(160,50,'-',WHITE,BLACK,3);
  tft.drawChar(220,50,'-',WHITE,BLACK,3);///Print for BAR
  tft.setFont(ARIAL_12);
  tft.setCursor(120,2);
  tft.setTextSize(1);
  tft.fillRect(0,0,320,20,YELLOW);
  tft.setTextColor(BLACK,YELLOW);
  tft.setCursor(70,2);
  tft.setTextSize(1);
  tft.print("ENTER THE  PASSWORD");

  /// Buttons to enter password
  tft.fillRoundRect(3,137,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(6,140,44,44,4,BLUE);
  tft.fillRoundRect(56,137,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(59,140,44,44,4,BLUE);
  tft.fillRoundRect(109,137,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(112,140,44,44,4,BLUE);
  tft.fillRoundRect(162,137,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(165,140,44,44,4,BLUE);
  tft.fillRoundRect(215,137,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(218,140,44,44,4,BLUE);
  tft.fillRoundRect(268,137,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(271,140,44,44,4,BLUE);
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  tft.fillRoundRect(3,190,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(6,193,44,44,4,BLUE);
  tft.fillRoundRect(56,190,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(59,193,44,44,4,BLUE);
  tft.fillRoundRect(109,190,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(112,193,44,44,4,BLUE);
  tft.fillRoundRect(162,190,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(165,193,44,44,4,BLUE);
  tft.fillRoundRect(215,190,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(218,193,44,44,4,BLUE);
  tft.fillRoundRect(268,190,50,50,4,WHITE); //fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  tft.fillRoundRect(271,193,44,44,4,BLUE);
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  tft.setTextColor(WHITE,BLUE);
  tft.setFont(ARIAL_8_N);
  tft.setCursor(21,155);
  tft.print("1");
  tft.setCursor(74,155);
  tft.print("2");
  tft.setCursor(127,155);
  tft.print("3");
  tft.setCursor(180,155);
  tft.print("4");
  tft.setCursor(233,155);
  tft.print("5");
  tft.setCursor(286,155);
  tft.print("<-");
  tft.setCursor(21,205);
  tft.print("6");
  tft.setCursor(74,205);
  tft.print("7");
  tft.setCursor(127,205);
  tft.print("8");
  tft.setCursor(180,205);
  tft.print("9");
  tft.setCursor(233,205);
  tft.print("0");
  tft.setCursor(286,205);
  tft.print("-");

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Display Current Time
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawTime(){
  fgColor = WHITE;
  tft.setFont(ARIAL_12_N);
  hex2bcd(Time.day());
  tft.drawChar(0,2,tens,fgColor,bgColor,1);
  tft.drawChar(12,2,ones,fgColor,bgColor,1);
  tft.drawChar(24,2,'-',fgColor,bgColor,1);
  hex2bcd(Time.month());
  tft.drawChar(36,2,tens,fgColor,bgColor,1);
  tft.drawChar(48,2,ones,fgColor,bgColor,1);
  tft.drawChar(60,2,'-',fgColor,bgColor,1);
  hex2bcd(Time.year());
  tft.drawChar(72,2,tens,fgColor,bgColor,1);
  tft.drawChar(84,2,ones,fgColor,bgColor,1);

  hex2bcd(Time.hour());
  tft.drawChar(108,2,tens,fgColor,bgColor,1);
  tft.drawChar(120,2,ones,fgColor,bgColor,1);
  tft.drawChar(132,2,':',fgColor,bgColor,1);
  hex2bcd(Time.minute());
  tft.drawChar(144,2,tens,fgColor,bgColor,1);
  tft.drawChar(156,2,ones,fgColor,bgColor,1);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Power Off either due to button press or sleep
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void powerOff(){
  if(digitalRead(WKP)){
    wkupTime++;
    if(wkupTime > 5){
      Sleep();
    }
  }
  else{
    wkupTime = 0;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Detect Touch
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void detectTouch(){
  TSPoint p = ts.getPoint();
  int z = p.z;
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (z < MINPRESSURE || z > MAXPRESSURE) {
     return;
  }
  int x = p.y;
  int y = p.x;
  // Scale from ~0->1000 to tft.width using the calibration #'s
  x = map(x, TS_MINX, TS_MAXX, 0, 320);
  y = map(y, TS_MINY, TS_MAXY, 0, 240);
  Serial.print("X = "); Serial.print(x);
  Serial.print("\tY = "); Serial.print(y);
  Serial.print("\tPressure = "); Serial.println(p.z);

  if((x>246) && (y>195)){
    if(menuState == 0){
      menuState = 1;
    }else{
      menuState = 0;
    }
    if(menuState) next  = 0;
  }
  if(menuState){
    if(((x > 10) && ( x < 60)) && ((y>35)&&(y<80))){
      next = 1;
    }
    if(((x > 90) && ( x < 150)) && ((y>35)&&(y<80))){
      next = 2;
    }
    if(((x > 160) && ( x < 220)) && ((y>35)&&(y<80))){
      next = 3;
    }
    if(((x > 255) && ( x < 300)) && ((y>35)&&(y<80))){
      next = 4;
    }
  }
  delay(500);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Detect Touch
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void detectTouchPassword(){
  TSPoint p = ts.getPoint();
  int z = p.z;
  if (z < MINPRESSURE || z > MAXPRESSURE) {
     return;
  }
  int x = p.y;
  int y = p.x;
  // Scale from ~0->1000 to tft.width using the calibration #'s
  x = map(x, TS_MINX, TS_MAXX, 0, 320);
  y = map(y, TS_MINY, TS_MAXY, 0, 240);
  if(((x > 3) && ( x < 53)) && ((y>137)&&(y<190))){
    tempPass[passNext] = 1;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;

  }
  if(((x > 56) && ( x < 106)) && ((y>137)&&(y<190))){
    tempPass[passNext] = 2;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 109) && ( x < 159)) && ((y>137)&&(y<190))){
    tempPass[passNext] = 3;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 162) && ( x < 212)) && ((y>137)&&(y<190))){
    tempPass[passNext] = 4;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 215) && ( x < 265)) && ((y>137)&&(y<190))){
    tempPass[passNext] = 5;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 268) && ( x < 318)) && ((y>137)&&(y<240))){
  //tempPass[tempNext] = 1;
    passNext--;
    tft.drawChar((40+(60*passNext)),50,'-',WHITE,BLACK,3);
  }
  if(((x > 3) && ( x < 53)) && ((y>190)&&(y<240))){
    tempPass[passNext] = 6;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 56) && ( x < 106)) && ((y>190)&&(y<240))){
    tempPass[passNext] = 7;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 109) && ( x < 159)) && ((y>190)&&(y<240))){
    tempPass[0] = 8;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 162) && ( x < 212)) && ((y>190)&&(y<240))){
    tempPass[passNext] = 9;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 215) && ( x < 265)) && ((y>190)&&(y<240))){
    tempPass[passNext] = 0;
    tft.drawChar((40+(60*passNext)),50,'*',WHITE,BLACK,3);
    passNext++;
  }
  if(((x > 268) && ( x < 318)) && ((y>190)&&(y<240))){
  //tempPass[tempNext] = 1;
    passNext--;
    tft.drawChar((40+(60*passNext)),50,'-',WHITE,BLACK,3);
  }
  delay(500);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug Screen on Serial Console
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void debugEvent(){
  String inString,tempString;
  char tempChar;
  int tempInt;
  int index[10];
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  if(Serial.available()){
    inString = Serial.readStringUntil('~');
  }
  ///////////////////////////////////RELAY///////////////////////////////////////
  if(inString == "PHI"){
    Serial.println("OK");
  }

///////////////////////////////////ABOUT///////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Hex to BCD Convertor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float measureTemp(){
  float tempReading = tc.readThermocoupleTemperature();
  //uint8_t fault =  tc.readFault();
  if (tempReading > 1760) {
    digitalWrite(MEASURE,LOW);
    digitalWrite(OPEN,HIGH);
    openFlag = HIGH;
    tc.begin();
    return 0;
  }
  else{
    digitalWrite(OPEN,LOW);
    openFlag = LOW;
    cjTemp = tc.readCJTemperature();
    return tempReading;
  }

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Hex to BCD Convertor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hex2bcd(int num){
    int a1,a2,a3,a4;
    tenthous = (num/10000)+48;
    a1 = num % 10000;
    thous = (a1/1000) + 48;
    a2 = a1%1000;
    hunds = (a2/100)+ 48;
    a3 = a2%100;
    tens = (a3/10) + 48;
    ones = (a3%10) + 48;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Draw Point
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawPoint(int x,int y, int pointSize,int pointColor){
 int i,j;
 for(i = 0; i<=pointSize; i++){
   for(j = 0;j<=pointSize;j++){
      tft.drawPixel(x+i,y+j,pointColor);
   }
 }
}
void readEEPROM(){

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Arduino map Function but as FLoat
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float mapf(float x, float in_min, float in_max, float out_min, float out_max){
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Heart Function
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void heartFunction(){

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Turn off everything and sleep
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sleep(){
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tc.oneShotTemperature();
  tft.setCursor(0,100);
  tft.setFont(ARIAL_8);
  tft.println("T U R N I N G    O F F   D E V I C E . . .");
  delay(2000);
  tft.fillScreen(BLACK);
  digitalWrite(BACKLIGHT,LOW);
  digitalWrite(POWERON,LOW);
  digitalWrite(HOLD,LOW);
  digitalWrite(OPEN,LOW);
  digitalWrite(MEASURE,LOW);
  WiFi.off();
  delay(1000);
  System.sleep(SLEEP_MODE_DEEP);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupts
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void timerISR(void)
{
 seconds++;
 heartFlag = !heartFlag;
 if(seconds > 300){
   sleepFlag = HIGH;
 }
}
