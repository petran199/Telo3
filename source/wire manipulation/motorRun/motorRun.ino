//i2c Master Code(HALL UNO)
#include <Wire.h>
#include <AFMotor.h>

AF_DCMotor motorL(1,MOTOR12_1KHZ);
AF_DCMotor motorR(2,MOTOR12_1KHZ);

// HAll sensor values first is Left Hall second is Right Hall
String first="";
String second="";
int leftHallDist = 0;
int rightHallDist = 0;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  delay(3000);
  setMotorSpeedLeftToRight(235,235);
}

void loop(){
  moveNplantThroughDesiredWidth(150);
  
  setMotorSpeedLeftToRight(0,0); //stop the engines
}

void moveNplantThroughDesiredWidth(uint actFieldLength){
  uint actualFieldDist = cmTohallDist(actFieldLength);
  while(leftHallDist <= actualFieldDist &&  rightHallDist <= actualFieldDist){
    Wire.requestFrom(5, 15);    // request 15 bytes from slave device #5
    hallDistCalc(); //calculate hall left and right  distance
    allignedMovementOfVehicle(); // try to align the movement of the vehicle
    // Serial.println("First:" + String(leftHallDist)); 
    // Serial.println("Second:" + String(rightHallDist));
  }
}
 uint cmTohallDist(uint actualFieldLeng){
    double tmp = (actualFieldLeng / 0,1172741); //1 pulse = 0.1172741cm
    uint cm = round(tmp);
    return cm;
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
    leftHallDist = first.toInt();
    cnt = 0;
    for(int i = 0; i<second.length();i++){
      if(!isDigit(second[i])){
        cnt++;
      }
    }
    second = second.substring(0,second.length()-cnt);
    rightHallDist = second.toInt();
}

void setMotorSpeedLeftToRight(byte left,byte right){
  motorR.setSpeed(right);
  motorL.setSpeed(left);
  motorR.run(FORWARD);
  motorL.run(FORWARD);
}

void allignedMovementOfVehicle(){
  if(leftHallDist - rightHallDist>=10){
    setMotorSpeedLeftToRight(100,235);
    delay(25);
  }else if(leftHallDist - rightHallDist<=10){
    setMotorSpeedLeftToRight(235,100);
    delay(25);
  }else{
    setMotorSpeedLeftToRight(235,235);
    delay(25);
  }
}


