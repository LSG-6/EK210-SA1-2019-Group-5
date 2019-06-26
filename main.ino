
#include "Statistic.h"
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <Wire.h>                                                 //include the I2C library to communicate with the sensor
#include "Adafruit_TCS34725.h"                                    //include the sensor library

#define laser 3                                                   //laser
#define Rbutton 4                                                 //button record
#define Nbutton 5                                                 //button next

#define commonAnode false 
byte gammatable[256]; 

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
                                                                  //Create an instance of the TCS34725 Sensor

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    //LCD config


Statistic ReferR;                                                 //Data storage
Statistic ReferG;
Statistic ReferB;
Statistic ReferL;
Statistic TestR;
Statistic TestG;
Statistic TestB;
Statistic TestL;

bool isReferrence;                                                //boolean for whether the pill is referrence pill
bool processing = false;                                          //boolean for whether a pill is being testing
bool LCDon = false;                                               //boolean for whether the LCD is showing result  
int state = 0;                                                    
bool pressed = false; 
int i = 0;

double Confiv[21] = {0.01,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.5,0.55,0.6,0.65,0.7,0.75,0.8,0.85,0.9,0.95,0.99};
double Tvalue[21] = {0.0126,0.063,0.1263,0.1901,0.2548,0.3204,0.3876,0.4566,0.5279,0.602,0.6796,0.7616,0.8492,0.9438,1.0478,1.1644,1.2994,1.4629,1.6772,2.0106,2.6822};


uint16_t ri, bi, gi, ci;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  if (tcs.begin()) {                                              //if the sensor starts correctly
    Serial.println("Found sensor");                               //print the happy message
  } 
  else {                                                        //if the sensor starts incorrectly
    Serial.println("No TCS34725 found ... check your connections");//print the not so happy message
    while (1); // halt!
  }

  pinMode(7,OUTPUT);

  pinMode(laser,OUTPUT);

  pinMode(Rbutton,INPUT_PULLUP);                                          //set "Record button" for input
  pinMode(Nbutton,INPUT_PULLUP);                                          //set "Next button" for input

  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
    
    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;      
    }
  }
  tcs.setInterrupt(false);                                          //turn on LED
 
  delay(100);                                                        //takes 60ms to read
 
  tcs.getRawData(&ci, &gi, &bi, &ci);                      //read the sensor
 
  tcs.setInterrupt(true);   

  
  lcd.begin(16, 2);
  lcd.print("Hello :D");
  lcd.setCursor(0,1);
  lcd.print("Press Red");
  //lcd.backlight();
  lcd.backlight();
}

void loop() {
  digitalWrite(7,HIGH);
  // put your main code here, to run repeatedly:
  //Serial.println("Loop begins");
  //Serial.println(digitalRead(Rbutton));
  //delay(100);
  
  switch(state){
    case 0:               //recording referrence pills
      isReferrence = true;
      break;
    case 1:               //recording testing pills
      isReferrence = false;
      break;
    case 2:               //displaying the result
      LCDon = true;
      break;
  }

  if(digitalRead(Rbutton) == LOW){                                          //button to start measurement
    processing = true;
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("Start recording");
    delay(2000);
    
  }

  if(digitalRead(Nbutton) == LOW && (!processing && !LCDon)){               //button to proceed to next step
    if(state != 3 && !pressed){
        state++;
        if(state == 1){
          lcd.clear();
          lcd.setCursor(0,0); 
          lcd.print("Change pill");
          lcd.setCursor(0,1); 
          lcd.print("Then press red");
          pressed = true;
        }
        if(state == 2){
          lcd.clear();
          lcd.setCursor(0,0); 
          lcd.print("Calculating");
        }
    }
    else if(state == 3 && !pressed){
      state = 0;
      pressed = true;
      lcd.clear();
      lcd.setCursor(0,0); 
      lcd.print("Now new test");
      lcd.setCursor(0,1); 
      lcd.print("Press red");
    }
  }
    
  if(processing && i < 25){
    if(i == 0){
      if(isReferrence){
        ReferR.clear();
        ReferG.clear();
        ReferB.clear();
        ReferL.clear();
      }
      else{
        TestR.clear();
        TestG.clear();
        TestB.clear();
        TestL.clear();
      }
    }
    
    uint16_t clear, red, green, blue, Lr, Lg, Lb;                              //declare variables for the colors
 
    tcs.setInterrupt(false);                                          //turn on LED
 
    delay(200);                                                        //takes 60ms to read
 
    tcs.getRawData(&red, &green, &blue, &clear);                      //read the sensor
 
    tcs.setInterrupt(true);                                           //turn off LED

    uint32_t sum = clear;
    float r, g, b;
    r = red; r /= sum;
    g = green; g /= sum;
    b = blue; b /= sum;
    r *= 256; g *= 256; b *= 256;
    Serial.print("r: ");  
    Serial.println(r);
    Serial.print("g: "); 
    Serial.println(g);
    Serial.print("b: "); 
    Serial.println(b);
    if(isReferrence){
      ReferR.add(r);
      ReferG.add(g);
      ReferB.add(b);
    }
    else{
      TestR.add(r);
      TestG.add(g);
      TestB.add(b);
    }

    digitalWrite(laser,HIGH);                                         //turn on laser
 
    delay(200);                                                        //takes 50ms to read
 
    tcs.getRawData(&Lr, &Lg, &Lb, &clear);                        //read the sensor
 
    digitalWrite(laser,LOW);                                          //turn off laser

    sum = clear;
    float l;
    l = Lr; l /= sum;
    l *= 256;
    
    Serial.print("l: ");
    Serial.println(l); 
    if(isReferrence){
      ReferL.add(l);
    }
    else{
      TestL.add(l);
    }

    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print(i+1);
    
    i++;
    if(i == 25){
      processing = false;
      i = 0;
      lcd.setCursor(0,0); 
      lcd.print("Finish!");
      lcd.setCursor(0,1);
      lcd.print("Press green");
      pressed = false;
    }
  }

  //Serial.print("state=");
  //Serial.println(state);
  if(LCDon){                                                          //display the result
    
    Serial.println("R: ");
    double RT = CalT(ReferR.average(),TestR.average(),CalSp(ReferR.unbiased_stdev(),TestR.unbiased_stdev()));
    Serial.println("G: ");
    double GT = CalT(ReferG.average(),TestG.average(),CalSp(ReferG.unbiased_stdev(),TestG.unbiased_stdev()));
    Serial.println("B: ");
    double BT = CalT(ReferB.average(),TestB.average(),CalSp(ReferB.unbiased_stdev(),TestB.unbiased_stdev()));
    Serial.println("L: ");
    double LT = CalT(ReferL.average(),TestL.average(),CalSp(ReferL.unbiased_stdev(),TestL.unbiased_stdev()));

    //Serial.println(j);
    Serial.println(RT);
    Serial.println(GT);  
    Serial.println(BT);
    Serial.println(LT);
    
    float RV = 0, GV = 0, BV = 0, LV = 0;
    for(int j = 0; j < 21; j++){                                      //match the t values to confidence value
      if(RT > Tvalue[j]){
        RV = Confiv[j];
      }
      if(GT > Tvalue[j]){
        GV = Confiv[j];
      }
      if(BT > Tvalue[j]){
        BV = Confiv[j];
      }
      if(LT > Tvalue[j]){
        LV = Confiv[j];
      }

      Serial.println(j);
      Serial.println(RV);
      Serial.println(GV);
      Serial.println(BV);
      Serial.println(LV);
    }

    
    //lcd.setCursor(0,0); 
    //lcd.print("Result incoming");
    //delay(2000);
    
    int result = ((RV+GV+BV+LV)/4)*100;                                   //show the result
   
    
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print(result);
    lcd.print("%");
    lcd.print(" sure diff");

    LCDon = false;
    pressed = false;
    state++;
    lcd.setCursor(0,1); 
    lcd.print("New test green");
  }

}

double CalSp(double s1, double s2){                                   //function to calculate Stand deviation
  Serial.print("s1: " );
  Serial.print(s1);
  Serial.print("  s2: ");
  Serial.print(s2);
  Serial.print("  result: ");
  Serial.println(sqrt(pow(s1,2)+pow(s2,2)));
  
  return sqrt((pow(s1,2)+pow(s2,2))/2);
  
}

double CalT(double y1, double y2, double sp ){                        //function to calcualte T values
  Serial.print("y1: " );
  Serial.print(y1);
  Serial.print("  y2: ");
  Serial.print(y2);
  Serial.print("  sp: ");
  Serial.print(sp);
  Serial.print("  sqrt(2/25): ");
  Serial.print("0.2828");
  Serial.print("  sp*sqrt(2/25): ");
  Serial.print(sp*0.2828);
  Serial.print("  abs(y1-y2): ");
  Serial.print(abs(y1-y2));
  Serial.print("  result: ");
  Serial.println((y1-y2)/(sp*0.2828));
  
  return abs(y1-y2)/(sp*0.2828);
  
}
