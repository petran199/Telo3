/* Main.ino */

/*-----( Import needed libraries )-----*/
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

/*-----( Defines and utiliities )-----*/
//      length lower-upper bounds
#define lengthLB 60
#define lengthUB 25000
//      width lower-upper bounds
#define widthLB 22
#define widthUB 12500
//      width lower-upper bounds
#define numbOfTreesLB 1
#define numbOfTreesUB 6

/*-----( Declare Constants )-----*/
// keypad constants
const byte ROWS = 4; // four Rows
const byte COLS = 4; // four Columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad


/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 16 chars 2 line display
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//
// check API at https://playground.arduino.cc/Code/Keypad#Download
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/*-----( Declare Variables )-----*/
unsigned int tmpVal(0);
unsigned int fieldLength(0);
char elegalCharsArray[] = {'B', '*', '#'};
String lcdQuestionsArray[5] = {
  "Give the desired length",
  "Give the desired width",
  "Give the desired number of trees",
  "Manually plant rate? Y(#)/N(*)",
  "Give the desired plant rate"
};
//1111 User Mode , 0000 Maintenance Mode
String passwords[] = {"1111", "0000"};
String mode[] = {"User", "Maintenance"};
String answearsArray[5] ;
String serialReadAnswear = "";
boolean runOnce = true;
boolean resetButtonPressed = false;
boolean stringComplete = false;
char tmpBuff[100];

/*-----( Declare Functions )-----*/

void clearSetCursorAt(byte AT_CHAR , byte AT_LINE );
void initialize();
void serialEvent();
void ckSerialToReadAns(String& serialReadAnswear);
void checkSerialAns(int& counter, String serialAns, unsigned int firstBound, unsigned int secondBound, String lengthWidthNumbTrees);
void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, unsigned int Delay = 0);
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, unsigned int msg1Delay = 0, unsigned int msg2Delay = 0);

/*----------( END OF DECLARATIONS ) ----------*/

void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  Serial.begin(9600);  // Used to type in characters
  lcd.begin(16, 2);
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
      lcd2LineMsg(sub1, sub2, 0, 0, 0, 1, 0, 2000);
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
      delay(1000);
      lcd.clear();
    }

  }
}/*--(end setup )---*/

void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{





}/* --(end main loop )-- */

/* ( Function Definition ) */
void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, unsigned int Delay = 0) {
  
  lcd.setCursor( cursorCharAt, cursorLineAt);
  lcd.print(msg);
  delay(Delay);
}
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, unsigned int msg1Delay = 0, unsigned int msg2Delay = 0) { //TODO write this function and continue for the lcs1LineMsg and commit to gitlab
  lcd.clear();
  lcd1LineMsg(firstMsg, msg1CursorCharAt, msg1CursorLineAt, msg1Delay);
  lcd1LineMsg(secondMsg, msg2CursorCharAt, msg2CursorLineAt, msg2Delay);
}

void checkSerialAns(int& counter, String serialAns, unsigned int firstBound, unsigned int secondBound, String lengthWidthNumbTrees) {
  // tmpAns exists to avoid decreasing counter twise
  bool tmpAns = false;
  if (serialAns.toInt() < firstBound || serialAns.toInt() > secondBound) {
    //check serial.length() in case user pressed enter without any value
    if (serialAns.length() < 1) {
      lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, 2000);
      --counter;
      tmpAns = true;
    }//end of serialAns.length()<1
    if (!tmpAns) {
      lcd2LineMsg(lengthWidthNumbTrees + ":" + serialReadAnswear + "cm", "Out of bounds...", 0, 0, 0, 1, 0, 2000);
      lcd.clear();
      lcd1LineMsg("Try again...",0,0,1500);
      // tmp is helpfull to turn the whole sentense that we wanna use into type String
      String lastSentenceToString = ((String)firstBound + "<" + lengthWidthNumbTrees + "<" + (String)secondBound);
      lcd2LineMsg("Bounds between",lastSentenceToString,0,0,0,1,0,2000);
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
  
  // if (Serial.available()) {
  //   // wait a bit for the entire message to arrive
  //   delay(10);
  //   // clear the screen
  //   lcd.clear();
  //   // read all the available characters
  //   while (Serial.available() > 0) {
  //     serialReadAnswear += (char)Serial.read();

  //   }
  //   lcd.write(serialReadAnswear.c_str());
  //   delay(1000);

  //   Serial.println(serialReadAnswear);
  //   serialReadAnswear = "";
  //   //      stringComplete = true;
  // }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    serialReadAnswear += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
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
    //  while (!passWordValid(serialReadAnswear)){ }
    //   }else{lcd.print("Serial problem");}
  }
}





bool passWordValid(char* serialReadAnsw ) {
  //  clearSetCursorAt();
  //  lcd.print("Give the password...");
  //  *serialReadAnswear = Serial.read();
  //  for(byte i = 0 ; i < (sizeof(passwords) / sizeof(passwords[0])); i++){
  //    if(*serialReadAnsw == *passwords[i]){
  //      clearSetCursorAt();
  //      lcd.print("You are loged in as ");
  //      lcd.setCursor(0,1);
  //      lcd.print(mode[i]);
  //      return true;
  //    }
  //  }
  //  clearSetCursorAt();
  //  lcd.print("Wrong password..");
  //  return false;
}



//  if (Serial.available()) {
//      // wait a bit for the entire message to arrive
//      delay(100);
//      // clear the screen
//      lcd.clear();
//      // read all the available characters
//      while (Serial.available() > 0) {
//        // display each character to the LCD
//        lcd.write(Serial.read());
//      }
//    }
//  Serial.begin(9600);  // Used to type in characters
//
//  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight

// ------- Quick 3 blinks of backlight  -------------
//  for(int i = 0; i< 3; i++)
//  {
//    lcd.backlight();
//    delay(250);
//    lcd.noBacklight();
//    delay(250);
//  }
//  lcd.backlight(); // finish with backlight on

//-------- Write characters on the display ------------------
// NOTE: Cursor Position: (CHAR, LINE) start at 0
//  lcd.setCursor(0,0); //Start at character 4 on line 0
//  lcd.print("Hello, world!");
//  delay(1000);
//  lcd.setCursor(0,1);
//  lcd.print("HI!YourDuino.com");
//  delay(8000);

// Wait and then tell user they can start the Serial Monitor and type in characters to
// Display. (Set Serial Monitor option to "No Line Ending")
//  lcd.clear();
//  lcd.setCursor(0,0); //Start at character 0 on line 0
//  lcd.print("Use Serial Mon");
//  lcd.setCursor(0,1);
//  lcd.print("Type to display");







//  Serial.print(serialReadAnswear);
//  delay(2000);
//  {
//    // when characters arrive over the serial port...
//    if (Serial.available()) {
//      // wait a bit for the entire message to arrive
//      delay(100);
//      // clear the screen
//      lcd.clear();
//      // read all the available characters
//      while (Serial.available() > 0) {
//        // display each character to the LCD
//        lcd.write(Serial.read());
//      }
//    }
//  }
