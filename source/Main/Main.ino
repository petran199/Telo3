/* @Main.ino
||  This is the Main.ino. In this file you can find code based on lcd, keypad ... libraries.
||  It is developed and maintained by Petros Chatzipetrou.
||  It is a project in frames of the course TELO_3.
||  It is started on 18.12.2017
*/

/*-----( Import needed libraries )-----*/
#include <Wire.h> // library for intercomunication of arduinos via sda and scl ports
#include <LiquidCrystal_I2C.h>// the lcd libray
#include <Keypad.h>// the keypad library
#include <AFMotor.h>// the motorshield library to move motor wheels and servo , stepper motors that are attached to it
/*----------( END_OF_IMPORT_LIBRARIES ) ----------*/

/*-----( #defines and utiliities )-----*/
typedef unsigned int uint;// create an shortcut about the desired type
#define LCD_CLEAR 1    // usefull for 1lineMsg if u want to use lcd.clear cmd or not
#define LCD_NO_CLEAR 0 // and this one if you dont want to use lcd.cler cmd
#define delay2K 2000   // 2k delay on a variety of functions
#define delay1K 1000   // 1k delay on a variety of functions
#define delay1_5K 1500 // 1.5k delay on a variety of functions
//      length lower-upper bounds
#define lengthLB 60
#define lengthUB 25000
//      width lower-upper bounds
#define widthLB 28
#define widthUB 12500
//      width lower-upper bounds
#define numbOfTreesLB 1
#define numbOfTreesUB 6
//
#define plantRateLB 6 // changed ean kaneis praxeis to min einai 6..
#define calcDistUB 25000 // uper bound of calculate distance 
#define passMaxChar 7 // max characters that user can fill up on keypad as password
/*----------( END_OF_#DEFINES_AND_UTILITIES_DECLARATIONS ) ----------*/

/*-----( Declare Constants )-----*/
const byte ROWS(4); // number of Rows of keypad
const byte COLS(4); // nuber of Columns of keypad
const byte numbOfQuestions(5); // number of the array in lcdQuestionsArray
const byte numbOfAnswears(5);  // number of the array in lcdAnswearsArray
/*----------( END_OF_CONSTANTS_DECLARATIONS ) ----------*/

/*-----( Declare Variables )-----*/
char keys[ROWS][COLS] = { // overview of the keypad keys that we need for object declaration
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {13, 12, 11, 10};        //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6, 5};            //connect to the column pinouts of the keypad
String lcdQuestionsArray[numbOfQuestions] = { // overview of Questions displayed to the User
    "Give the desired Length",
    "Give the desired Width",
    "Give the desired number of trees",
    "Manually _plant_ rate? Y(#)/N(*)",
    "Give the desired plant rate"};
String lcdAnswearsArray[numbOfAnswears] = {""}; // here we will keep the answears of User
String keypadReadAnswear = "";                // initialise the var that keeps temporar answear of User
String tmpPass = "";    // used to turn the numbers that user press on keypad to asterisk (*)
String userPassAns = "";
String passwords[] = {"8888", "0000"};        //1111 User Mode , 0000 Maintenance Mode
String mode[] = {"User mode", "Maintenance mode"};
bool isEnterActive(false);        // check if enter is pressed
String lcdTypingSentences[numbOfQuestions] = { //used in some functions related to the question above
    "Length:", "Width:", "Num of trees:", "Your Answear:", "Pl. rate:"};
uint lowerBounds[numbOfQuestions] = {lengthLB, widthLB, numbOfTreesLB, 1, plantRateLB}; 
uint upperBounds[numbOfQuestions] = {lengthUB, widthUB, numbOfTreesUB, 1, 0};         //the last 0 value is calculated on the way  
uint totalRounds(0);       //Number of courses in Specifications 2.2.5
uint actualFieldLength(0); //Length of courses in Specifications 2.2.5
uint plantRate(0);  
// HAll sensor values first is Left Hall second is Right Hall
String first="";
String second="";
int leftHallDist = 0;
int rightHallDist = 0;
// Hygrometer sensor stuff
int hydroSensorPin = A0;
byte hydroValuePercent(0);
//Led photoresistor stuff..
byte ledL = 4;
byte ledR = 3;
int photoresistorPin = A1;
//Encoder Hall Sensor stuff
const byte encoder0pinA = 2;//A pin -> the interrupt pin 0
const byte encoder0pinB = 1;//B pin -> the digital pin 8
int encoder0PinALast;
volatile int duration0(0);//the number of the pulses
boolean Direction0(true);//the rotation direction 
/*----------( END_OF_VARIABLE_DECLARATIONS ) ----------*/

/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 16 chars 2 line display
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// check API at https://playground.arduino.cc/Code/Keypad#Download
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
AF_DCMotor motorL(1,MOTOR12_1KHZ);// left motor
AF_DCMotor motorR(2,MOTOR12_1KHZ);//right motor
/*----------( END_OF_OBJECT_DECLARATIONS ) ----------*/

/*-----( Declare Functions )-----*/
void (*resetFunc)(void) = 0; //this is the software reset that looks at addr 0x00
//write to screen 1 line of message(message, cursot at position,cursor at line,how long to be displayed,clear lcd before run this func or no)
void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, uint Delay, bool lcdClear);
//write to screen 2 line of messages, it's based on lcd1LineMsg func almost same arguments
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, uint msg1Delay, uint msg2Delay);
//check the users answear about the bounds of length width plant rate etc..
void ckKeypadAns(int& i, String keypadAns, uint firstBound, uint secondBound, String boundsAns);
void ckPlantRate(int& i); // handle plant rate question based on user's response.Manual or Auto?
void ckCalcDist(int& i); //calculates totalRounds and if it is out of bounds it prints msg and reset.. 
void questionMsg(int& i);// prints each time the question that are stored on lcdQuestionsArray
void init0();// simple initialization like serial.begin, wire begin, lcd.begin etc..
void checkAndPrintQuestion(int i,char key);// checks the user's answear based on the question of the system and prints them out . also informs user when he writes too many chars
void swOnQuestionKeyPress(int i,char key);// switch on every keypad key press and take actions
void checkQuestion(int& i);//checks if user pushed a button as answear  and then runs ckKeypadAns func
void checkAndPrintPass(byte Passlength,char key);// do the same work as checkAndPrintQuestion but based on password rules this time
void swOnPassKeyPress(char key);// the same rules as swOnQuestionKeyPress but for password actions
void checkPassword();//the same as checkQuestion and then checks if user put the correct passwords
void checkModeAndPrintMsg();// check mode based on user's pasword and print apropriate msg to screen
void moveNplantThroughDesiredFieldLength(uint actFieldLength);// move the car till the end of actualFieldLeng and do the tree planting on the way 
uint cmToHallDist(uint actualFieldLeng);// conver  centimeters to Hall measure system (pulses)
void hallDistCalc(); // reads the response of hall slave arduino and put its values to first, second, leftHallDist, rightHallDist vars
void setMotorSpeedLeftToRight(byte left,byte right);// set the speed of left and right motor
void allignedMovementOfVehicle();// based on  hallSensors' values, trying to stabilise the difference of them to be equal to zero and move the car in straight line
byte cnvHydroValToPercent(int sensorValOfHydro);// converts the the sensor value of Hydrometer to percentage
void checkHydroValue();//checks for hydrometer values and if it's Greater than 50 continues else prints warning msg
void ckLightsOfCar(); // checks if there is a need to turn the lights on or off in the begining
void handlePassAndPrintMsg();// check the user's password and print apropriate msg to screen
void printQAndCkUserResponse();//prints the questions and check  the user answear to be between bounds and continue
void finalMsgAndCLoseScreen(); // prints the final msg to user about the plant rate and the lets start.. then after a while closes the screen
/*----------( END_OF_FUNCTION_DECLARATIONS ) ----------*/

void setup() /*----( SETUP: RUNS ONCE )----*/
{
  init0();
  //  while(1){
  //   setMotorSpeedLeftToRight(235,235);
  //  }
    //initialization of screen, pins etc..
   ckLightsOfCar();// check photoresistor to turn lights on
   handlePassAndPrintMsg();// handles the user keypresses and print msg to screen
   checkModeAndPrintMsg();//chekcs user or maintenance mode and take apropriate actions based on requirments
   checkHydroValue();// checks Hydro and print msg
   printQAndCkUserResponse(); // start writing the answears on lcd and check user answears
   finalMsgAndCLoseScreen(); //prints the plant rate and lets start msg and then closes the screen
} /*--(end setup )---*/

void loop() /*----( LOOP: RUNS CONSTANTLY )----*/
{
  // moveNplantThroughDesiredFieldLength(actualFieldLength);//move through user's desired field length
  
  // setMotorSpeedLeftToRight(0,0); //stop the engines afthe completion
} /* --(end main loop )-- */

/* ( Function Definition ) */
void wheelSpeed0()//TODO check to see whats wrong with hall measurements
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
  Serial.println("Right:"+String(duration0));
}

void EncoderInit0()
{
  Direction0 = true;//default -> Forward  
  pinMode(encoder0pinB,INPUT);  
  attachInterrupt(0, wheelSpeed0, CHANGE);
}

void printQAndCkUserResponse(){
  for (int i = 0; i < (sizeof(lcdQuestionsArray) / sizeof(lcdQuestionsArray[0])); i++)
  {
    questionMsg(i);
    while(!isEnterActive){
      if (kpd.getKeys())
      {
        for (int j=0; j<LIST_MAX; j++)   // Scan the whole key list.
        {
          if ( kpd.key[j].stateChanged )   // Only find keys that have changed state.
          {
            switch (kpd.key[j].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              swOnQuestionKeyPress(i,kpd.key[j].kchar);
            break;
            }
          }
        }
      }
    }
    checkQuestion(i);
  }
}

void finalMsgAndCLoseScreen(){
  lcd1LineMsg("Let's start...", 0, 1, delay2K * 2, LCD_NO_CLEAR);
  lcd.clear();
  lcd.off();
}

void handlePassAndPrintMsg(){
  lcd2LineMsg("Give the pass...","Password:",0,0,0,1,0,0);
  while(keypadReadAnswear!=passwords[0] && keypadReadAnswear!= passwords[1]){
    while(!isEnterActive){
      if (kpd.getKeys())
      {
        for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        {
            if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
            {
              switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
              case PRESSED:
                 swOnPassKeyPress(kpd.key[i].kchar);
              break;
              }
            }
          }
      }
    }
    checkPassword();
  }
}

void checkHydroValue(){
  hydroValuePercent = cnvHydroValToPercent(analogRead(hydroSensorPin));
  if(hydroValuePercent>=50){
    lcd1LineMsg(("humidity is:"+String(hydroValuePercent)+"%"),0,0,delay1K,LCD_CLEAR);
  }
  while(hydroValuePercent<50){
   lcd1LineMsg(("humidity is:"+String(hydroValuePercent)+"%"),0,0,delay2K,LCD_CLEAR);
   lcd2LineMsg("humidity is low","Water the ground",0,0,0,1,0,delay1K);
   hydroValuePercent = cnvHydroValToPercent(analogRead(hydroSensorPin));
  }
  lcd2LineMsg("Bingo!!!",("humidity is:"+String(hydroValuePercent)+"%"),0,0,0,1,0,delay2K);
}

byte cnvHydroValToPercent(int sensorValOfHydro){
  byte percVal = map(sensorValOfHydro,430,1023,100,0);
  return percVal;
}

void checkModeAndPrintMsg(){
  lcd1LineMsg("Bingo!",0,0,0,LCD_CLEAR);
  userPassAns = keypadReadAnswear;
  delay(delay1K);
  for (int i = 0; i<(sizeof(passwords) / sizeof(passwords[0])); i++){
    if(userPassAns==passwords[i]){
      lcd2LineMsg("Welcome to",mode[i],0,0,0,1,0,delay2K);
    }
  }
  keypadReadAnswear="";
  if(userPassAns == passwords[1]){
    //TODO move all motors
    while(1){}
  }
}

void checkQuestion(int& i){
  if(keypadReadAnswear.length()<=0){
    lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, delay2K);
    keypadReadAnswear = "";
    isEnterActive = false;
    --i;
  }else {
      if(i==3){
        ckPlantRate(i);
      }else{
        ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
      }
  }
}

void checkPassword(){
  if(keypadReadAnswear.length()<=0){
    lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, delay2K);
    keypadReadAnswear = "";
    tmpPass = "";
    lcd2LineMsg("Give the pass...","Password:",0,0,0,1,0,0);
    isEnterActive = false;
  }else {
    if(keypadReadAnswear!=passwords[0] && keypadReadAnswear!= passwords[1]){
      isEnterActive = false;
      lcd1LineMsg("Wrong Pass...", 0, 0, delay1_5K, LCD_CLEAR);
      lcd1LineMsg("Try again...", 0, 0, delay1_5K, LCD_CLEAR);
      keypadReadAnswear = "";
      tmpPass = "";
      lcd2LineMsg("Give the pass...","Password:",0,0,0,1,0,0);
    }
  }
}

void checkAndPrintQuestion(int i,char key){
  if(keypadReadAnswear.length()<String(upperBounds[i]).length()){
    keypadReadAnswear+=key;
    lcd1LineMsg((lcdTypingSentences[i]+keypadReadAnswear),0,0,0,LCD_CLEAR);
  }else{
    lcd2LineMsg("Too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
    lcd1LineMsg((lcdTypingSentences[i]+keypadReadAnswear),0,0,0,LCD_CLEAR);
  }
}

void checkAndPrintPass(byte Passlength,char key){
  if(keypadReadAnswear.length()<Passlength){
    tmpPass = "";
    keypadReadAnswear+=key;
    for(int i=0;i<keypadReadAnswear.length(); i++){
      tmpPass+="*";
    }
    lcd2LineMsg("Give the pass...",("Password:"+tmpPass),0,0,0,1,0,0);
  }else{
    lcd2LineMsg("Too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
    lcd2LineMsg("Give the pass...",("Password:"+tmpPass),0,0,0,1,0,0);
  }
}

void swOnPassKeyPress(char key){
  if(isDigit(key)){
    checkAndPrintPass(passMaxChar,key);
  }else if(isAlpha(key)){
    switch(key){
      case 'A':
      isEnterActive = true;
      break;
      case 'C':
      if(keypadReadAnswear.length()>0){
        tmpPass = "";
        keypadReadAnswear = keypadReadAnswear.substring(0,keypadReadAnswear.length()-1);
        for(int i=0;i<keypadReadAnswear.length(); i++){
          tmpPass+="*";
        }
        lcd2LineMsg("Give the pass...",("Password:"+tmpPass),0,0,0,1,0,0);
      }
      break;
      case 'D':
      lcd1LineMsg("Reset...", 0, 0, delay1_5K, LCD_CLEAR);
      resetFunc();
      break;
    }
  }else{
    checkAndPrintPass(passMaxChar,key);
  }
}

void swOnQuestionKeyPress(int i,char key){
  if(isDigit(key) ){
    if(i!=3){
      checkAndPrintQuestion(i,key);
    }
  }else if(isAlpha(key)){
    switch(key){
      case 'A':
      isEnterActive = true;
      break;
      case 'C':
      if(keypadReadAnswear.length()>0){
        keypadReadAnswear = keypadReadAnswear.substring(0,keypadReadAnswear.length()-1);
        lcd1LineMsg((lcdTypingSentences[i]+keypadReadAnswear),0,0,0,LCD_CLEAR);
      }
      break;
      case 'D':
      lcd1LineMsg("Reset...", 0, 0, delay1_5K, LCD_CLEAR);
      resetFunc();
      break;
    }
  }else{
    if(i==3){
      if(keypadReadAnswear.length()<String(upperBounds[i]).length()){
        keypadReadAnswear+=key;
        if(keypadReadAnswear=="#"){
          lcdAnswearsArray[i] = "Yes";
        }else{lcdAnswearsArray[i] = "No";}
        lcd1LineMsg((lcdTypingSentences[i]+lcdAnswearsArray[i]),0,0,0,LCD_CLEAR);
      }else{
        lcd2LineMsg("Too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
        lcd1LineMsg((lcdTypingSentences[i]+lcdAnswearsArray[i]),0,0,0,LCD_CLEAR);
      }
    }
  }
}

void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, uint Delay = 0, bool lcdClear = LCD_NO_CLEAR)
{
  if (lcdClear)
  {
    lcd.clear();
  };
  lcd.setCursor(cursorCharAt, cursorLineAt);
  lcd.print(msg);
  delay(Delay);
}

void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, uint msg1Delay = 0, uint msg2Delay = 0)
{ 
  lcd1LineMsg(firstMsg, msg1CursorCharAt, msg1CursorLineAt, msg1Delay, LCD_CLEAR);
  lcd1LineMsg(secondMsg, msg2CursorCharAt, msg2CursorLineAt, msg2Delay);
}

void init0(){
  Serial.begin(9600);                   // Used to type in characters
  Wire.begin();
  lcd.begin(16, 2);                     // initialize the dimensions of display
  digitalWrite(ledL, LOW);//lights off
  digitalWrite(ledR, LOW);//lights off
  // EncoderInit0();
}

void questionMsg(int& i){
  isEnterActive = false;
  String sub1 = lcdQuestionsArray[i].substring(0, 16);
  String sub2 = lcdQuestionsArray[i].substring(16);
  sub2.trim();//trims the second sentence to avoid white spaces
  // Prints out the questions
  lcd2LineMsg(sub1, sub2, 0, 0, 0, 1, 0, delay2K);
  lcd1LineMsg(lcdTypingSentences[i], 0, 0, 0, LCD_CLEAR);
}

void ckCalcDist(int& i){
  totalRounds = round(lcdAnswearsArray[i].toFloat() / 28);
  uint calcDist = (totalRounds * actualFieldLength) + ((totalRounds - 1) * 28);
  if (calcDist > calcDistUB)
  { //total distance is out of bounds
    lcd2LineMsg("Total Distance", "Out of bounds", 0, 0, 0, 1, 0, delay2K);
    lcd1LineMsg("Reset...", 0, 0, delay1K, LCD_CLEAR);
    resetFunc();
  }
  else
  {
    lcd2LineMsg("The robot car", "Goes through", 0, 0, 0, 1, 0, delay2K);
    lcd2LineMsg((String)calcDist + "cm along", "the field", 0, 0, 0, 1, 0, delay2K);
  }
}

void ckPlantRate(int& i){
  float calc = actualFieldLength * totalRounds; //auto calc for plant rate
  plantRate = round(calc / lcdAnswearsArray[i - 1].toInt());
  upperBounds[i + 1] = plantRate; // add these value in case user wants manual plant rate
  keypadReadAnswear = ""; // clean keypad in case user goes to manual plant rate
  if (lcdAnswearsArray[i] == "No")
  {
    lcd1LineMsg("Plant rate:" + (String)plantRate, 0, 0, 0, LCD_CLEAR);
    ++i; //  jump to the next itteration and avoid last question
  }
}

void ckKeypadAns(int& i, String keypadAns, uint firstBound, uint secondBound, String boundsAns)
{
  // tmpAns exists to avoid decreasing i twise
  bool tmpAns = false;
  boundsAns = boundsAns.substring(0, boundsAns.length() - 1);
  if (keypadAns.toInt() < firstBound || keypadAns.toInt() > secondBound)
  {
    //check serial.length() in case user pressed enter without any value
    if (keypadAns.length() < 1)
    {
      lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, delay2K);
      tmpAns = true;
    } //end of serialAns.length()<1
    else /* (!tmpAns) */
    {
      String tmp = (boundsAns + ":" + keypadAns);
      lcd2LineMsg(/* (keypad.buff[0] == 2 || keypad.buff[0] == 4) ? */ tmp /* : (tmp + "cm") */, "Out of bounds...", 0, 0, 0, 1, 0, delay2K);
      lcd1LineMsg("Try again...", 0, 0, delay1_5K, LCD_CLEAR);
      // tmp is helpfull to turn the whole sentense that we wanna use into type String
      String lastSentenceToString = (String(firstBound) + "<" + boundsAns + "<" + String(secondBound));
      // lastSentenceToString.co
      Serial.print(lastSentenceToString);
      lcd2LineMsg("Bounds between", lastSentenceToString, 0, 0, 0, 1, 0, delay2K);
    }
    --i;
    keypadReadAnswear = "";
    lcd.clear();
  } // end of serialAns.toInt()<firstBound || serialAns.toInt()>secondBound
  else
  { // serialAns bounds OK!
    lcdAnswearsArray[i] = keypadAns;//save the answear to the array
    keypadReadAnswear = "";
    if(i==0){
      actualFieldLength = lcdAnswearsArray[i].toInt() - 22;
    }else if(i==1){
      ckCalcDist(i);
    }else if(i==3){
      ckPlantRate(i);
    }else{}
  } // end of serialAns bounds OK!
}

void moveNplantThroughDesiredFieldLength(uint actFieldLength){
  uint actualFieldDist = cmToHallDist(actFieldLength);
  Serial.println(actualFieldDist); 
  while(leftHallDist <= actualFieldDist &&  rightHallDist <= actualFieldDist){
    Wire.requestFrom(5, 15);    // request 15 bytes from slave device #5
    hallDistCalc(); //calculate hall left and right  distance
    allignedMovementOfVehicle(); // try to align the movement of the vehicle
    // Serial.println("First:" + String(leftHallDist)); 
    // Serial.println("Second:" + String(rightHallDist));
  }
}

 uint cmToHallDist(uint actualFieldLeng){
  //  float tmp1 = actualFieldLeng;
    float tmp =((float)actualFieldLeng  / (float) 0,1172741) ; //1 pulse = 0.1172741cm, float type cast is required in order to avoid warnings of the compiler
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

void ckLightsOfCar(){
  if(analogRead(photoresistorPin)<=50){
    digitalWrite(ledL, HIGH);
    digitalWrite(ledR, HIGH);

  }
}