#include <DS1302.h>
#include <TFT_ILI9163C.h>
#include <DHT.h>

//------------- TFT ISP 1.44  --------------

#define BLACK   0x0000
//#define BLUE    0x001F
#define RED     0xF800
//#define GREEN   0x07E0
//#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
//#define WHITE   0xFFFF

#define BTN_SELECT 7
#define BTN_PLUSE 6//a6
#define BTN_MINUS 12

#define __CS 10
#define __DC 9
#define __RES 8

// LED - 3.5v
// SCK - SCK(pin 13)
// SDA - MOSI (pin 11)
// A0 -  pin 9
// RESET - pin 8
// CS -  pin 10
// GND - GND
// VCC - 5v
TFT_ILI9163C display = TFT_ILI9163C(__CS, __DC, __RES);

// ------------- DHT11 -----------------------

#define DHTTYPE DHT11
DHT dht(2, DHTTYPE);

// ------------------------- RTC -------------

// DS1302:  CE pin    -> Arduino Digital 4
//          I/O pin   -> Arduino Digital 5
//          SCLK pin  -> Arduino Digital 6
#define CE_PIN 4  //rst
#define SCK_PIN 5 //dat
#define IO_PIN 6  //clk
DS1302 rtc(CE_PIN,SCK_PIN,IO_PIN);
Time time = rtc.time();

char degree_symbol = char(128);
char space_symbol = char(139);

void setup() {
  //Serial.begin(9600);
  //dht.begin();
  rtc.writeProtect(false);
  rtc.halt(false);
  
  display.begin();
  display.setRotation(2);
  display.fillScreen(BLACK);
  display.setTextSize(3);
  pinMode(3, OUTPUT);
}

byte prev_h = -1;
byte prev_t = -1;

byte editIndex = 0;

boolean needRefreshTime = false;
boolean needRefreshDate = false;

void loop() {

  int light = analogRead(1);
  analogWrite(3, map(light, 0, 1023, 0, 100) );
  
  if(editIndex == 0) {
    time = rtc.time();
  }
  checkEdit();
  printTime();
  
  int h = (int)dht.readHumidity();
  int t = (int)dht.readTemperature();
  //if (!isnan(h) && !isnan(t)) {
    if(prev_h != h || prev_t != t) {
      prev_h = h;
      prev_t = t;
      display.setTextSize(4);
      prindText(String(t) + degree_symbol + 'C', 15, 65, YELLOW);
      prindText(String(h) + '%', 15, 100, MAGENTA);
    }
  //}
  
  delay(50);
}

void checkEdit() {
  if(digitalRead(BTN_SELECT)) {
    editIndex++;
    if(editIndex > 6) {
      editIndex = 0;
    }

    if(editIndex == 1) {//H
      display.drawFastHLine(3, 27, 32, RED);
    } else if(editIndex == 2) {//Min
      display.drawFastHLine(3, 27, 32, BLACK);
      display.drawFastHLine(57, 27, 32, RED);
    } else if(editIndex == 3) {// sec
      display.drawFastHLine(57, 27, 32, BLACK);
      display.drawFastHLine(103, 27, 22, RED);
    } else if(editIndex == 4) {//date
      display.drawFastHLine(103, 27, 32, BLACK);
      display.drawFastHLine(3, 53, 21, RED);
    } else if(editIndex == 5) {//month
      display.drawFastHLine(3, 53, 21, BLACK);
      display.drawFastHLine(38, 53, 21, RED);
    } else if(editIndex == 6) {//year
      display.drawFastHLine(38, 53, 21, BLACK);
      display.drawFastHLine(74, 53, 46, RED);
    } else if(editIndex == 0) {
      display.drawFastHLine(74, 53, 46, BLACK);
      rtc.time(time);
    }
    //Serial.print("editIndex=");
    //Serial.println(editIndex);
    delay(300);
  } else {
    boolean btnPluse = analogRead(BTN_PLUSE) > 500;
    boolean btnMinus = digitalRead(BTN_MINUS) == HIGH;
    
    if(editIndex > 0 && (btnPluse || btnMinus)) {
      if(editIndex == 1) {//H
        time.hr = nextValue(time.hr, 0, 23, btnPluse);
      } else if(editIndex == 2) {//Min
        time.min = nextValue(time.min, 0, 59, btnPluse);
      } else if(editIndex == 3) {// sec
        time.sec = nextValue(time.sec, 0, 59, btnPluse);
      } else if(editIndex == 4) {//date
        time.date = nextValue(time.date, 1, 31, btnPluse);
      } else if(editIndex == 5) {//month
        time.mon = nextValue(time.mon, 1, 12, btnPluse);
      } else {//year
        time.yr = nextValue(time.yr, 2015, 2070, btnPluse);
      }
      //Serial.print("change editIndex=");
      //Serial.println(editIndex);
      needRefreshTime = editIndex < 4;
      needRefreshDate = editIndex > 3;
      delay(300);
    }
  }
}

int nextValue(int current, int minVal, int maxVal, boolean orderAsc) {
  if(orderAsc) {
    current++;
  } else {
    current--;
  }
  if(current > maxVal) {
    return minVal;
  } else if(current < minVal) {
    return maxVal;
  }
  return current;
};

void prindText(String text, int x, int y, int color) {
  display.setCursor(x,y);
  int l = text.length();
  display.setTextColor(BLACK);
  for(int i = 0; i < l; i++) {
    display.print(space_symbol);
  }
  display.setCursor(x,y);
  display.setTextColor(color);
  display.print(text);
}


byte mins = 100;
byte day = 0;
byte sec = 100;

void printTime(){
  if(sec != time.sec || needRefreshTime || needRefreshDate) {
    sec = time.sec;
    if (mins != time.min || needRefreshTime) {
      char timeStr[6];
      mins = time.min;
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", time.hr, time.min);
      display.setTextSize(3);
      prindText(timeStr, 2, 2, 0xFFFF);
      //Serial.println("printTime");
    }
  
    char secsStr[4];
    snprintf(secsStr, sizeof(secsStr), ":%02d", time.sec);
    display.setTextSize(2);
    prindText(secsStr, 91, 9, 0xFFFF);
    if (day != time.day || needRefreshDate) {
      day = time.day;
      char dateStr[11];
      snprintf(dateStr, sizeof(dateStr), "%02d/%02d/%4d", time.date, time.mon, time.yr);
      prindText(dateStr, 2, 35, 0xAAAA);
    }
  }
  needRefreshTime = false;
  needRefreshDate = false;
}
