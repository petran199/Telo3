//i2c Master Code(HALL UNO)
#include <Wire.h>
//-----------------------Diloseis Hall-----------------------------------//
const byte encoder0pinA = 2;//A pin -> the interrupt pin 0
const byte encoder0pinB = 4;//B pin -> the digital pin 8

String tmp="";// save temporar values of hall values

//const byte encoder0pinC = 7;//A pin -> the interrupt pin 1
//const byte encoder0pinD = 5;//B pin -> the digital pin 9

int encoder0PinALast;
//int encoder1PinALast;

volatile int duration0(0);//the number of the pulses
boolean Direction0(true);//the rotation direction 

//volatile int duration1(0);//the number of the pulses
//boolean Direction1(true);//the rotation direction 
//-----------------------------------------------------------------------//


void setup()
{
  Wire.begin(5);
  Serial.begin(9600);
  EncoderInit0();//Initialize the module0
//  EncoderInit1();//Initialize the module1
//  Wire.onRequest(requestHandler);
}

void loop(){
//  interrupts();    // or use sei(); to Enables interrupts
//delay (1000);     //Wait 1 second
//noInterrupts();
Serial.println("Left:"+String(duration0));
//Serial.println("Right:"+String(duration1));
//delay(1000);

}

//void requestHandler(){
//  tmp = (String(duration0)+","+String(duration1));
//  const char* c = tmp.c_str();
////  Serial.print(c);
////  Serial.println();
//  Wire.write(c);
//
//}
//------------------------Sinartiseis Hall Sensors---------------------------

void EncoderInit0()
{
  Direction0 = true;//default -> Forward  
  pinMode(encoder0pinB,INPUT);  
  attachInterrupt(0, wheelSpeed0, CHANGE);
}

//void EncoderInit1()
//{
//  Direction1 = true;//default -> Forward  
//  pinMode(encoder0pinD,INPUT);  
//  attachInterrupt(1, wheelSpeed1, CHANGE);
//}
// 
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
//Serial.println("Left:"+String(duration0));
}
//
//void wheelSpeed1()
//{
//  int Lstate1 = digitalRead(encoder0pinC);
//  if((encoder1PinALast == LOW) && Lstate1==HIGH)
//  {
//    int val1 = digitalRead(encoder0pinD);
//    if(val1 == LOW && Direction1)
//    {
//      Direction1 = false; //Reverse
//    }
//    else if(val1 == HIGH && !Direction1)
//    {
//      Direction1 = true;  //Forward
//    }
//  }
//  encoder1PinALast = Lstate1;
// 
//  if(!Direction1)  duration1++;
////  else  duration1--;
//}
