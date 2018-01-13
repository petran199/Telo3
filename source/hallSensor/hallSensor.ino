/* @hallSensor.ino
||  This is the hallSensor.ino. In this file you can find code based on Hall encoder Sensor.
||  It is developed and maintained by Petros Chatzipetrou. Contributors are Sgouros Xristos and Nikos Kagasidis
||  It is a project in frames of the course TELO_3.
||  It is started on 18.12.2017
*/

/*-----( Import needed libraries )-----*/
#include <Wire.h>

/*-----( Declare Variables )-----*/
const byte encoderpinA = 2;//A pin -> the interrupt pin 0
const byte encoderpinB = 4;//B pin -> the digital pin 4
String tmp="";// save temporary the value of the duration 
int encoderPinALast(0);// we save the last state of encoderpinA
volatile int duration0(0);//the total number of the pulses
boolean Direction0(true);//the rotation direction 

/*----( SETUP: RUNS ONCE )----*/
void setup() 
{
  Wire.begin(5);
  Serial.begin(115200);
  EncoderInit();//Initialize the module
  Wire.onRequest(requestHandler);
}

/*----( LOOP: RUNS CONSTANTLY )----*/
void loop(){

}

/* ---------- ( Function Definition )----------- */

/* 
@function
@abstract save the duration to tmp variable  and then send it to the master via wire comunication.
*/
void requestHandler(){
 tmp = (String(duration));
 const char* c = tmp.c_str();
 Wire.write(c);

}

/* 
@function
@abstract Initialise the pinModes and attach an interrupt service routine to encoder pin A
*/
void EncoderInit()
{
  Direction = true;//default -> Forward  
  pinMode(encoderpinB,INPUT);  
  attachInterrupt(digitalPinToInterrupt(encoderpinA), wheelSpeed, CHANGE);
}

/* 
@function
@abstract checks for the direction of the vehicle and depends on that, increase or decrease duration variable. By doing so, we calculate the pulses of the hall encoder sensor.
*/
void wheelSpeed()
{
  int Lstate = digitalRead(encoderpinA);
  if((encoderPinALast == LOW) && Lstate==HIGH)
  {
    int val = digitalRead(encoderpinB);
    if(val == LOW && !Direction)
    {
      Direction = true; //Reverse
    }
    else if(val == HIGH && Direction)
    {
      Direction = false;  //Forward
    }
  }
  encoderPinALast = Lstate;
  if(!Direction)  duration++;
  else  duration--;
  duration++;
}
