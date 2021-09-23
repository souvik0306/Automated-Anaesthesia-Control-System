#include "MAX30100_PulseOximeter.h"
#include <U8g2lib.h>
#include <Wire.h>

#define REPORTING_PERIOD_MS     500
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

const int numReadings=10;
float filterweight=0.5;
uint32_t tsLastReport = 0;
uint32_t last_beat=0;
int readIndex=0;
int average_beat=0;
int average_SpO2=0;
bool calculation_complete=false;
bool calculating=false;
bool initialized=false;
byte beat=0;

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int tmp = A0;
const int p = 8;
int enA = 9;
int in1 = 8;
int in2 = 7;

int value = 0;

const int ledPin = 13;//led is connected to digital pin 13
const int ledPin1 = 10;
const int ledPin2 = 6;
int pMin = 0;
int pMax = 1023;
int sen1=0;
int sen2=0;


int reading = 0;
int reading_final;
int Heart_rate;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
  show_beat();
  last_beat=millis();
}

void show_beat() 
{
  u8g2.setFont(u8g2_font_cursor_tf);
  u8g2.setCursor(8,10);
  if (beat==0) {
    u8g2.print("_");
    beat=1;
  } 
  else
  {
    u8g2.print("^");
    beat=0;
  }
  u8g2.sendBuffer();
}

void initial_display() 
{
  if (not initialized) 
  {
    u8g2.clearBuffer();
    show_beat();
    u8g2.setCursor(24,12);
    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf);
    u8g2.print("Place finger");  
    u8g2.setCursor(0,30);
    u8g2.print("on the sensor");
    u8g2.sendBuffer(); 
    initialized=true;
  }
}

void display_calculating(int j)
{
  if (not calculating) {
    u8g2.clearBuffer();
    calculating=true;
    initialized=false;
  }
  show_beat();
  u8g2.setCursor(24,12);
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf);
  u8g2.print("Measuring"); 
  u8g2.setCursor(0,30);
  for (int i=0;i<=j;i++) {
    u8g2.print(". ");
  }
  u8g2.sendBuffer();
}
void display_values()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf);
 
  u8g2.setCursor(65,12);  
  u8g2.print(average_beat);
  u8g2.print(" Bpm");
  u8g2.setCursor(0,30);
  u8g2.print("SpO2 ");
  u8g2.setCursor(65,30);  
  u8g2.print(average_SpO2);
  u8g2.print("%"); 
  u8g2.sendBuffer();
}

void calculate_average(int beat, int SpO2) 
{
  if (readIndex==numReadings) {
    calculation_complete=true;
    calculating=false;
    initialized=false;
    readIndex=0;
    display_values();
  }
  
  if (not calculation_complete and beat>30 and beat<220 and SpO2>50) {
    average_beat = filterweight * (beat) + (1 - filterweight ) * average_beat;
    average_SpO2 = filterweight * (SpO2) + (1 - filterweight ) * average_SpO2;
    readIndex++;
    display_calculating(readIndex);
  }
}

void setup()
{
    Serial.begin(115200);
    //Serial.begin(9600);
    u8g2.begin();
    pox.begin();
    pox.setOnBeatDetectedCallback(onBeatDetected);
    initial_display();

     lcd.begin(16, 2);
     //Serial.begin(9600);
     pinMode(11, OUTPUT);
     pinMode(p, OUTPUT);
    // put your setup code here, to run once:
    // Set all the motor control pins to outputs
    pinMode(enA, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    
    // Turn off motors - Initial state
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    
    pinMode(A3,INPUT);//initialize sensor1 as input
    pinMode(A1 ,INPUT);
    pinMode(ledPin,OUTPUT);//initialize led as output
    pinMode(ledPin1,OUTPUT);
    pinMode(ledPin2,OUTPUT);
}

void controls()
{
  // Make sure to call update as fast as possible
    pox.update();
    if ((millis() - tsLastReport > REPORTING_PERIOD_MS) and (not calculation_complete)) {
        calculate_average(pox.getHeartRate(),pox.getSpO2());
        tsLastReport = millis();
    }
    if ((millis()-last_beat>5000)) {
      calculation_complete=false;
      average_beat=0;
      average_SpO2=0;
      initial_display();
    }
}

void loop()
{
    // Make sure to call update as fast as possible
    pox.update();
    if ((millis() - tsLastReport > REPORTING_PERIOD_MS) and (not calculation_complete)) {
        calculate_average(pox.getHeartRate(),pox.getSpO2());
        tsLastReport = millis();
    }
    if ((millis()-last_beat>5000)) {
      calculation_complete=false;
      average_beat=0;
      average_SpO2=0;
      initial_display();
    }

    digitalWrite(p,LOW);
  int Temp = analogRead(tmp);
  float volts = (Temp / 965.0) * 5;
  float c = (volts - .5) * 100;
  float f = (c * 9 / 5 + 32)-11;
  lcd.setCursor(0, 0);
  lcd.print("Temp = ");
  lcd.print(f);
  lcd.print(" F");
  delay(1000);
  if(f >= 99)
  {
  motor_run();
  }
  else
  { 
    // Now turn off motors
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  controls();
  
  lcd.clear();
  
  
  // put your main code here, to run repeatedly:
  
  
  for(int i=0;i<5;i++)                   //For debugging.
      {
          reading = reading + analogRead(A3);
          //calculating sum of ambient junk readings 5 times so that we can eliminate them afterwards.
      }
      reading_final = (reading)/5;           // Average junk reading calulated.
      delay(100);
      Heart_rate = analogRead(A3)-reading_final;  // Final reading value.
      Serial.println(Heart_rate);                 //Printing and plotting.
 controls();
 sen1 = analogRead(A3);
 sen2 = analogRead(A1);
 sen1= map(sen1, pMin, pMax, 0,250);
 sen2= map(sen2, pMin, pMax, 0,250);
  
  
  lcd.setCursor(0, 0);
  lcd.print("Sys=");
  lcd.print(sen1);
  lcd.setCursor(9, 0);
  lcd.print("Dia=");
  lcd.print(sen2);
   delay(1000);
  controls(); 
    if((sen1>130 && sen1<=135)&&(sen2<100 && sen2>=50))//25 psi to 35 psi is normal pressure
   {
   
  lcd.setCursor(0,1);
  lcd.print("BP Pre-hyper");
  delay(1500);
  digitalWrite(ledPin2, HIGH);  
  delay(1000);
 
   }
      if((sen1<=130 && sen1>=110)&&(sen2<90 && sen2>=50))
   {
   
  lcd.setCursor(0,1);
  lcd.print("BP Normal");
     
  delay(1500);
  digitalWrite(ledPin2, HIGH);
  delay(1000);
  }
     if((sen1>135 && sen1<145) && (sen2>=50 && sen2<110))
   {
   
  lcd.setCursor(0,1);
  lcd.print("BP S1 Hyper");
     
  delay(1500);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  }
    
    if((sen1<80 && sen2<65)||(sen2<=49))
       {
         lcd.setCursor(0,1);
  lcd.print("BP Hypo");
     
  delay(1500);
  digitalWrite(ledPin1, HIGH);
  delay(1000);
        }
  if((sen1>=145 && sen1<180) && (sen2>=60 && sen2<120))
      {
         lcd.setCursor(0,1);
  lcd.print("BP S2 Hyper");
     
  delay(1500);
  digitalWrite(ledPin, HIGH);
  delay(1000);
        }
  if(sen1>=180 || sen2>=120)
      {
         lcd.setCursor(0,1);
  lcd.print("BP Hyper Crisis");
    delay(1500);
  digitalWrite(ledPin, HIGH);
  delay(1000);
        }     
  else
  { 
  
   lcd.clear();  
   digitalWrite(ledPin, LOW);
  }

  controls(); 
  
  digitalWrite(ledPin, LOW);
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  
  controls();
}

void motor_run() 
{
  
  // For PWM maximum possible values are 0 to 255
  analogWrite(enA, 255);
  
  // Turn on motors
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);      
}
