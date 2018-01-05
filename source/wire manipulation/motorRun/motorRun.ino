//i2c Master Code(HALL UNO)
#include <Wire.h>
#include <AFMotor.h>

AF_DCMotor motorL(1,MOTOR12_1KHZ);
AF_DCMotor motorR(2,MOTOR12_1KHZ);

// HAll sensor values first is Left Hall second is Right Hall
String first="";
String second="";

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  delay(3000);
  setMotorSpeedLeftToRight(235,235);
}

void loop(){
  while(first.toInt() <= 1330 && second.toInt() <= 1330){
    Wire.requestFrom(5, 15);    // request 15 bytes from slave device #5
    hallDistCalc(); //calculate hall left and right  distance
  }
  setMotorSpeedLeftToRight(0,0);
}

void hallDistCalc(){
  first = "";
  second = "";
    while(Wire.available())    // slave may send less than requested
    { 
      
      char c = Wire.read();
      if(c!=','){
        first+=c;
      }else{
         while(Wire.available()){
          c=Wire.read();
          second+=c;
         }
      }
    }
    byte cnt = 0;
    for(int i = 0; i<first.length();i++){
      if(!isDigit(first[i])){
        cnt++;
      }
    }
    first = first.substring(0,first.length()-cnt);
    Serial.println("First:"+first);
    // first = "";
    cnt = 0;
    for(int i = 0; i<second.length();i++){
      if(!isDigit(second[i])){
        cnt++;
      }
    }
    second = second.substring(0,second.length()-cnt);
    Serial.println("Second:"+second);
    // second = "";
    
    delay(50);
}

void setMotorSpeedLeftToRight(byte left,byte right){
  motorR.setSpeed(right);
  motorL.setSpeed(left);
  motorR.run(FORWARD);
  motorL.run(FORWARD);
}



