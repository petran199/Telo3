/* @Main.ino
||  This is the Main.ino. In this file you can find code based on lcd, keypad ... libraries.
||  It is developed and maintained by Petros Chatzipetrou.
||  It is a project in frames of the course TELO_3.
||  It is started on 18.12.2017
*/

/*-----( Import needed libraries )-----*/

#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

/*----------( END_OF_IMPORT_LIBRARIES ) ----------*/

/*-----( #defines and utiliities )-----*/

typedef unsigned int uint;

#define LCD_CLEAR 1 // usefull for 1lineMsg if u want to use lcd.clear cmd or not
#define LCD_NO_CLEAR 0 // and this one if you dont want to use lcd.cler cmd 

#define delay2K 2000 // 2k delay on a variety of functions
#define delay1K 1000 // 1k delay on a variety of functions
#define delay1_5K 1500 // 1.5k delay on a variety of functions

//      length lower-upper bounds
#define lengthLB 60
#define lengthUB 25000
//      width lower-upper bounds
#define widthLB 22
#define widthUB 12500
//      width lower-upper bounds
#define numbOfTreesLB 1
#define numbOfTreesUB 6

/*----------( END_OF_#DEFINES_AND_UTILITIES_DECLARATIONS ) ----------*/

/*-----( Declare Constants )-----*/
// keypad constants
const byte ROWS(4); // number of Rows of keypad
const byte COLS(4); // nuber of Columns of keypad

const byte numbOfQuestions(5); // number of the array in lcdQuestionsArray 
const byte numbOfAnswears(5); // number of the array in lcdAnswearsArray

/*----------( END_OF_CONSTANTS_DECLARATIONS ) ----------*/

/*-----( Declare Variables )-----*/

char keys[ROWS][COLS] = {// overview of the keypad keys that we need for object declaration
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad
String lcdQuestionsArray[numbOfQuestions] = { // overview of Questions displayed to the User
  "Give the desired length",
  "Give the desired width",
  "Give the desired number of trees",
  "Manually plant rate? Y(#)/N(*)",
  "Give the desired plant rate"
};
String lcdAnswearsArray[numbOfAnswears] = { } ; // here we will keep the answears of User
String serialReadAnswear = "";// initialise the var that keeps temporar answear of User
String passwords[] = {"1111", "0000"}; //1111 User Mode , 0000 Maintenance Mode
String mode[] = {"User", "Maintenance"};
char elegalCharsArray[] = {'B', '*', '#'};

/*----------( END_OF_VARIABLE_DECLARATIONS ) ----------*/

/*-----( Declare objects )-----*/

// set the LCD address to 0x27 for a 16 chars 2 line display
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//
// check API at https://playground.arduino.cc/Code/Keypad#Download
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/*----------( END_OF_OBJECT_DECLARATIONS ) ----------*/

/*-----( Declare Functions )-----*/

void clearSetCursorAt(byte AT_CHAR , byte AT_LINE );
void initialize();
// checks if serial is available and read seiral monitor
void ckSerialToReadAns(String& serialReadAnswear);
// check if serial is available and then check the bounds of each question 
void checkSerialAns(int& counter, String serialAns, uint firstBound, uint secondBound, String lengthWidthNumbTrees);

void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, uint Delay, bool lcdClear );
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, uint msg1Delay , uint msg2Delay );

void keypadEvent(KeypadEvent key); // the system goes through this code every time a key is pressed
bool isNumb(KeypadEvent key); // check if the letter is number or not
bool isAlpha(KeypadEvent key); // check if the letter is Alpha or not


/*----------( END_OF_FUNCTION_DECLARATIONS ) ----------*/



void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  Serial.begin(9600);  // Used to type in characters
  lcd.begin(16, 2); // initialize the dimensions of display
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
  // start writing the answears on lcd
  for (int i = 0; i < (sizeof(lcdQuestionsArray) / sizeof(lcdQuestionsArray[0])); i++) {
    lcd.clear();
    if (lcdQuestionsArray[i].length() > 16) {
      String tmpStr = lcdQuestionsArray[i];
      String sub1 = tmpStr.substring(0, 16);
      String sub2 = tmpStr.substring(16);
      //trims the second sentence to avoid white spaces
      sub2.trim();
      // Prints out the questions
      //          Msg1,Msg2,lcdPos ,delay
      lcd2LineMsg(sub1, sub2, 0, 0, 0, 1, 0, delay2K);
      // w8 till Serial is available
      while (!Serial.available()) {};
      if (Serial.available()) {
        //checks Serial and read/write all values to var serialReadAnswear
        ckSerialToReadAns(serialReadAnswear);
        switch (i) {
          case 0:
            //checks for bounds and take apropriate descision
            checkSerialAns(i, serialReadAnswear, lengthLB, lengthUB, "Length");
            break;
          case 1:
            checkSerialAns(i, serialReadAnswear, widthLB, widthUB, "Width");
            break;
          case 2:
            checkSerialAns(i, serialReadAnswear, numbOfTreesLB, numbOfTreesUB, "Numb trees");
            break;
          case 3:
            //TODO write smth to chech manually plant rate Y/N
            break;
          default:
          //TODO write smth else or find the lower-upper bounds
            //checkSerialAns(i, serialReadAnswear, lengthLB, lengthUB, "Plant rate");
            break;
        }
        delay(100);
        // just empty the var to begin with new question
        serialReadAnswear = "";
      }
    } else {
      lcd.print(lcdQuestionsArray[i]);
      delay(delay1K);
      lcd.clear();
    }

  }
}/*--(end setup )---*/

void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{





}/* --(end main loop )-- */

/* ( Function Definition ) */
bool isNumb(KeypadEvent key){
  for(byte i(0); i < ROWS; i++){
    for(byte j(0); j < COLS-1; j++){
      if(keys[i][j] == key){
        // test if it works like that else replace  with break and tmpVars
        return true;
      }
    }
  }
  return false;
};
bool isAlpha(KeypadEvent key){
  for(byte i(0); i < ROWS; i++){
    for(byte j(COLS-1); j < COLS; j++){
      if(keys[i][j] == key){
        // test if it works like that else replace  with break and tmpVars
        return true;
      }
    }
  }
  return false;
};
void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()){
  case PRESSED:
      if (key == isNumb(key)) {
          if(key != '*' && key != '#'){
            //concat the key to serialReadAnswear
            serialReadAnswear += (char)key;
          }else if(key == '*'){
            //TODO smth with asterisk letter fo keypad
          }else{// key == '#'
            //TODO smth with # symbol 
          }
      }else if(key == isAlpha(key)){
        //TODO do smth to hadnle letters
        switch (key){
          case 'A':
          break;
          case 'B':
          break;
          case 'C':
          break;
          case 'D':
          break;
          default: //IN case somth goes to this option catch it
          break;
        }
      }
      break;
  // case RELEASED: // these are useless right now
      // if (key == '#') {
      //   // digitalWrite(ledPin,!digitalRead(ledPin));
      //   // ledPin_state = digitalRead(ledPin);        // Remember LED state, lit or unlit.
      // }
  //     break;
  // case HOLD:
  //     break;
  }
}

void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, uint Delay = 0, bool lcdClear = LCD_NO_CLEAR ) {
  if(lcdClear){
    lcd.clear();
  };
  lcd.setCursor( cursorCharAt, cursorLineAt);
  lcd.print(msg);
  delay(Delay);
}
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, uint msg1Delay = 0, uint msg2Delay = 0) { //TODO write this function and continue for the lcs1LineMsg and commit to gitlab
  lcd1LineMsg(firstMsg, msg1CursorCharAt, msg1CursorLineAt, msg1Delay, LCD_CLEAR);
  lcd1LineMsg(secondMsg, msg2CursorCharAt, msg2CursorLineAt, msg2Delay);
}

void checkSerialAns(int& counter, String serialAns, uint firstBound, uint secondBound, String lengthWidthNumbTrees) {
  // tmpAns exists to avoid decreasing counter twise
  bool tmpAns = false;
  if (serialAns.toInt() < firstBound || serialAns.toInt() > secondBound) {
    //check serial.length() in case user pressed enter without any value
    if (serialAns.length() < 1) {
      lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, delay2K);
      --counter;
      tmpAns = true;
    }//end of serialAns.length()<1
    if (!tmpAns) {
      lcd2LineMsg(lengthWidthNumbTrees + ":" + serialReadAnswear + "cm", "Out of bounds...", 0, 0, 0, 1, 0, delay2K);
      lcd1LineMsg("Try again...", 0, 0, delay1_5K, LCD_CLEAR);
      // tmp is helpfull to turn the whole sentense that we wanna use into type String
      String lastSentenceToString = ((String)firstBound + "<" + lengthWidthNumbTrees + "<" + (String)secondBound);
      lcd2LineMsg("Bounds between", lastSentenceToString, 0, 0, 0, 1, 0, delay2K);
      --counter;
    }
    lcd.clear();
  }// end of serialAns.toInt()<firstBound || serialAns.toInt()>secondBound
  else { // serialAns bounds OK!

  }// end of serialAns bounds OK!
}

void ckSerialToReadAns(String& serialReadAnswear) {
  // wait a bit for the entire message to arrive
  delay(10);
  // clear the screen
  lcd.clear();
  // read all the available characters
  while (Serial.available() > 0) {
    serialReadAnswear += (char)Serial.read();
  }
  //detracked null char '\0' at the end of the sentence
  serialReadAnswear = serialReadAnswear.substring(0, serialReadAnswear.length() - 1);
  delay(10);
}



void clearSetCursorAt(byte AT_CHAR = 0, byte AT_LINE = 0) {
  lcd.clear();
  lcd.setCursor(AT_CHAR, AT_LINE);
}

void initialize() {

  clearSetCursorAt();
  //  lcd.print("Give password...");
  //  delay(500);
  Serial.print(Serial.available());
  if (Serial.available()) {
    clearSetCursorAt(0, 0);
    while (Serial.available() > 0) {
      //      serialReadAnswear = serialReadAnswear + Serial.read();
      serialReadAnswear.concat(Serial.read());
      lcd.write(Serial.read());
    }
    delay(500);
    //    while(Serial.available()>0){
    //    lcd.write(Serial.read());
    //    }

    for (byte i = 0 ; i < (sizeof(passwords) / sizeof(passwords[0])); i++) {
      Serial.print(serialReadAnswear);
      if (serialReadAnswear == passwords[i]) {
        clearSetCursorAt();
        lcd.print("You are loged in as ");
        lcd.setCursor(0, 1);
        lcd.print(mode[i]);
        break;
      } else {
        clearSetCursorAt();
        delay(500);
        lcd.print("Wrong password..");
        delay(500);
      }
    }
  }
}



