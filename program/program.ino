//*****************Libraries used*****************
#include <Wire.h>
#include <RTClib.h>
#include "U8glib.h"

RTC_DS3231 rtc;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

//****************Initializations********************************

int       i,j;                                 //For loop values
int       initialValue = 0;                   //Value for spectrum
int       sensorValue;                        //Get A0 value
int       value[100];                         //Samples

const int buttonPin         = 3;              //Interupt touch pin for help()
const int relayPin          = 6;              //Relay control pin
const int relayPin2         = 7;              //2nd Relay control pin
float     current           = 0;              //Current value converted from analogRead
float     currentValue      = 0;              //Final Current Value
float     voltage           = 0;              //Voltage value converted from analogRead
float     voltageValue      = 0;              //Final Voltage Value
int       p                 = 0;              //Value used to define charging and discharging function
int       percentage;                         //Battery percentage 
int       count             = 0;              //To fix intial value after 10 values
int       count2            = 0;              //To find if full charge
volatile bool flag = LOW;                     //Bool value to define usual or help function
volatile bool flag2 = HIGH;                   //To take intial value;

//******************Function Calls*********************
void discharging();
void charging();
void background();
void battery();
void help();
void pageHi();
void pageWelcome();
void interrupt();

void (*pages[])() = { pageHi, pageWelcome};

////////////////*********************************************************************************************///////////////////

void setup () {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  
//**************Setup nitializations******************
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

    if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  
  attachInterrupt(digitalPinToInterrupt(buttonPin), interrupt, RISING);
  digitalWrite(relayPin, LOW);
  digitalWrite(relayPin2, LOW);

//*********************Starting Pages*********************
  for(i =0; i<2; i++) {
  u8g.firstPage();  
  do {
    (*pages[p])();
  } while( u8g.nextPage() );
  delay(2000);
  p = p+1;
  }
}

///////////////*********************************************************************************************//////////////////

void loop() {
//  digitalWrite(relayPin, HIGH);
//  digitalWrite(relayPin2, HIGH);
  int value2;
  int value3;
  float voltage5V = 0;

  voltage = 0;
  p = 0;
  for(j=0; j<100; j++) {
    value2 = analogRead(A1);
    sensorValue = analogRead(A2);
    value[j] = sensorValue - initialValue;
    if(flag2 == HIGH && count == 5) {  initialValue = sensorValue; flag2 = LOW;}
//    Serial.print(sensorValue);
//    Serial.print("     ");
//    Serial.println(initialValue);
    voltage5V = value2 * 0.0216;                  
    voltage = voltage + voltage5V;
    if(value[j] > 15) { value[j] = 15; }
    if(value[j] < -15) { value[j] = -15; }
  }
  count++;
  voltageValue = voltage/100;
//  digitalWrite(2, HIGH);
  
///******************OLED***************************** 
     u8g.firstPage();                                   
  do {
          background();
          
      //    Serial.println(value1);
      //    Serial.println(value2);
      //    Serial.println(initialValue);
      //    Serial.println(voltage5V);
      //    Serial.println(voltageValue);
      //    Serial.println(percentage);
          
          if (flag == LOW) {
                int value = analogRead(A0);
                if(value < 100) { discharging();}                      //discharge and charge decision
                else { charging();}
          }
          else { help(); }
  } while(u8g.nextPage());
  delay(10);
}

///////////////*********************************************************************************************//////////////////

//************Function called during charging**************
void charging() {
  digitalWrite(relayPin, LOW);
  percentage = (28.571 * voltageValue) - 257.14;
  if(percentage >= 100) { 
    count2++;
    if(count2 >= 200) {
      while(1) {
        digitalWrite(relayPin2, HIGH);
        u8g.setFont(u8g_font_9x15Br);
        u8g.drawStr( 15, 40, "CHARGE FULL");
      }
    } 
  }
  else { 
    digitalWrite(relayPin2, LOW);
    u8g.setFont(u8g_font_9x15Br);
    u8g.drawStr( 15, 40, "CHARGING..");
    u8g.setPrintPos(50, 55); 
    u8g.print(percentage);
    } 
}

//***********Function called during discharging************
void discharging() {  
  digitalWrite(relayPin, HIGH);                               
  percentage = (28.571 * voltageValue) - 257.14;
  if (percentage > 99) { percentage = 100; }
  if (percentage < 1) { percentage = 0; }

  u8g.drawLine(30, 38, 120, 38);
      for (j=0; j<90; j+=5) {
        u8g.drawLine(30+j, 38, 30+j, 38+value[j]);
        u8g.drawLine(30+j+1, 38, 30+j+1, 38+value[j]);
        u8g.drawLine(30+j+2, 38, 30+j+2, 38+value[j]);
      }
  battery();
}

//************Help function with contact details***********
void help() {                                           
  u8g.setFont(u8g_font_6x10);
  u8g.drawStr( 5, 25, "FOR HELP");
  u8g.drawStr( 5, 35, "+91 7981677757");
  u8g.drawStr( 5, 45, "+91 8123402840");
  u8g.setFont(u8g_font_04b_03);
  u8g.drawStr( 5, 55, "stephenstephen664@gmail.com");
}

//************Main background fucction with RTC*****************
void background() {                                     
  DateTime now = rtc.now();
  char buf4[] = "DD-MM-YYYY DDD hh:mm";
  Serial.println(now.toString(buf4));
  u8g.setFont(u8g_font_6x10);
  u8g.drawFrame(2,1,126,63);
  u8g.drawStr( 6, 12, buf4); //date and time
}

//*****************Battery function for battery percentage and current*******************
void battery() {
  int a = (-0.3 * percentage)+30; 
  Serial.println(a);
  u8g.drawFrame(5,20,15,35);              // battery frame
  u8g.drawBox(8,17,9,4);                  //battery top small box
  u8g.drawBox(7,22+a,11,31-a);            // battery percentage inside box
  u8g.setFont(u8g_font_04b_03);
  u8g.setPrintPos(5, 62); 
  u8g.print(percentage);
  u8g.drawStr( 20, 62, "%");
//  Serial.println(percentage);
}

//***********Interrupt for initial two pages written in setup()*******************
void interrupt() {                                      
  flag = !flag;
//  Serial.println("Interrupt");
  delay(50);
}

//********************"Hi" page in setup()*******************************
void pageHi() {                                         
  u8g.setFont(u8g_font_9x15Br);
  u8g.drawStr( 60, 15, "Hi");
  u8g.drawStr( 35, 32, "Earaddy");
  u8g.drawStr( 35, 60, "Kavitha");
  u8g.setFont(u8g_font_6x10);
  u8g.drawStr( 65, 45, "&");
  
}

//********************"Welcome" page in setup()************************
void pageWelcome() {                                    
  u8g.setFont(u8g_font_9x15Br);
  u8g.drawStr( 33, 20, "Welcome");
  u8g.drawStr( 26, 45, "Veeresh");
  u8g.drawStr( 17, 60, "Music World");
  u8g.setFont(u8g_font_6x10);
  u8g.drawStr( 60, 32, "to");
}
