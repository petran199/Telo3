//i2c Master Code(HALL UNO)
#include <Wire.h>
//-----------------------Diloseis Hall-----------------------------------//
const byte encoder0pinA = 2;//A pin -> the interrupt pin 0
const byte encoder0pinB = 4;//B pin -> the digital pin 8

String tmp="";// save temporar values of hall values

//const byte encoder0pinC = 7;//A pin -> the interrupt pin 1
//const byte encoder0pinD = 5;//B pin -> the digital pin 9

int encoder0PinALast(0);
//int encoder1PinALast;

volatile int duration0(0);//the number of the pulses
boolean Direction0(true);//the rotation direction 

//volatile int duration1(0);//the number of the pulses
//boolean Direction1(true);//the rotation direction 
//-----------------------------------------------------------------------//


void setup()
{
  Wire.begin(5);
  Serial.begin(115200);
  EncoderInit0();//Initialize the module0
//  EncoderInit1();//Initialize the module1
 Wire.onRequest(requestHandler);
}

void loop(){

delay(10);

}

void requestHandler(){
 tmp = (String(duration0));//+","+String(duration1));
 const char* c = tmp.c_str();
//  Serial.print(c);
//  Serial.println();
 Wire.write(c);

}
//------------------------Sinartiseis Hall Sensors---------------------------

void EncoderInit0()
{
  Direction0 = true;//default -> Forward  
  pinMode(encoder0pinB,INPUT);  
  attachInterrupt(digitalPinToInterrupt(encoder0pinA), wheelSpeed0, CHANGE);
}

void wheelSpeed0()
{
  int Lstate0 = digitalRead(encoder0pinA);
  if((encoder0PinALast == LOW) && Lstate0==HIGH)
  {
    int val0 = digitalRead(encoder0pinB);
    if(val0 == LOW && !Direction0)
    {
      Direction0 = true; //Reverse
    }
    else if(val0 == HIGH && Direction0)
    {
      Direction0 = false;  //Forward
    }
  }
  encoder0PinALast = Lstate0;
  if(!Direction0)  duration0++;
  else  duration0--;
  duration0++;
// Serial.println("Left:"+String(duration0));
}
