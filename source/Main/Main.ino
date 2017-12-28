/* @Main.ino
||  This is the Main.ino. In this file you can find code based on lcd, keypad ... libraries.
||  It is developed and maintained by Petros Chatzipetrou.
||  It is a project in frames of the course TELO_3.
||  It is started on 18.12.2017
*/

/*-----( Import needed libraries )-----*/

#include <Wire.h> // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

/*----------( END_OF_IMPORT_LIBRARIES ) ----------*/

/*-----( #defines and utiliities )-----*/

typedef unsigned int uint;

#define LCD_CLEAR 1    // usefull for 1lineMsg if u want to use lcd.clear cmd or not
#define LCD_NO_CLEAR 0 // and this one if you dont want to use lcd.cler cmd

#define CONTAINS_CM 1
#define NO_CM 0

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
// keypad constants
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
byte colPins[COLS] = {9, 8, 7, 6};            //connect to the column pinouts of the keypad
String lcdQuestionsArray[numbOfQuestions] = { // overview of Questions displayed to the User
    "Give the desired Length",
    "Give the desired Width",
    "Give the desired number of trees",
    "Manually _plant_ rate? Y(#)/N(*)",
    "Give the desired plant rate"};
String lcdAnswearsArray[numbOfAnswears] = {""}; // here we will keep the answears of User
String keypadReadAnswear = "";                // initialise the var that keeps temporar answear of User
String tmpPass = "";    // used to turn the numbers that user press on keypad to asterisk (*)
String passwords[] = {"1111", "0000"};        //1111 User Mode , 0000 Maintenance Mode
String mode[] = {"User", "Maintenance"};
bool isEnterActive(false);        // check if enter is pressed
String lcdTypingSentences[numbOfQuestions] = { //used in some functions related to the question above
    "Length:", "Width:", "Num of trees:", "Your Answear:", "Pl. rate:"};
uint lowerBounds[numbOfQuestions] = {lengthLB, widthLB, numbOfTreesLB, 1, plantRateLB}; 
uint upperBounds[numbOfQuestions] = {lengthUB, widthUB, numbOfTreesUB, 1, 0};         //the last 0 value is calculated on the way  

uint totalRounds(0);       //Number of courses in Specifications 2.2.5
uint actualFieldLength(0); //Length of courses in Specifications 2.2.5
uint plantRate(0);  

/*----------( END_OF_VARIABLE_DECLARATIONS ) ----------*/

/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 16 chars 2 line display
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// check API at https://playground.arduino.cc/Code/Keypad#Download
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/*----------( END_OF_OBJECT_DECLARATIONS ) ----------*/

/*-----( Declare Functions )-----*/
void (*resetFunc)(void) = 0; //this is the software reset that looks at addr 0x00
void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, uint Delay, bool lcdClear);
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, uint msg1Delay, uint msg2Delay);
// void keypadEvent(KeypadEvent key); // the system goes through this code every time a key is pressed
// bool isNumb(KeypadEvent key);      // check if the letter is number or not
// // bool isAlpha(KeypadEvent key);     // check if the letter is Alpha or not

void ckKeypadAns(int& i, String keypadAns, uint firstBound, uint secondBound, String boundsAns);

void concatKeysAndPrint(char key,bool containsCm);

void warnTooManyChars(String msg,bool containsCm , uint delay ); 
void ckPlantRate(int& i);
void ckCalcDist(int& i);
void questionMsg(int& i);
void init0();
void checkAnswears(int& i);
void checkAndPrintPass(byte Passlength,char key);
void swOnPassKeyPress(char key);
void checkPassword();

/*----------( END_OF_FUNCTION_DECLARATIONS ) ----------*/

void setup() /*----( SETUP: RUNS ONCE )----*/
{
   init0(); //initialization
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
  lcd1LineMsg("Bingo!",0,0,0,LCD_CLEAR);
  keypadReadAnswear="";
  delay(delay2K);
    
  // start writing the answears on lcd
  for (int i = 0; i < (sizeof(lcdQuestionsArray) / sizeof(lcdQuestionsArray[0])); i++)
  {
    questionMsg(i);
    
    while(!isEnterActive){
      // ckAndHandleKeyPress(i,swOnQuestionKeyPress);
      if (kpd.getKeys())
      {
        for (int j=0; j<LIST_MAX; j++)   // Scan the whole key list.
        {
          if ( kpd.key[j].stateChanged )   // Only find keys that have changed state.
          {
            switch (kpd.key[j].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              //  swOnPassKeyPress(kpd.key[i].kchar);
              swOnQuestionKeyPress(i,kpd.key[j].kchar);
            break;
            }
          }
        }
      }
    }
    checkQuestion(i);
 
  }
  lcd1LineMsg("Let's start...", 0, 1, delay2K * 3, LCD_NO_CLEAR);
  lcd.clear();
} /*--(end setup )---*/

void loop() /*----( LOOP: RUNS CONSTANTLY )----*/
{

} /* --(end main loop )-- */

/* ( Function Definition ) */

void ckAndHandleKeyPress(int& i,void (*swOnKeyPress)(int , char)){
  if (kpd.getKeys())
  {
    for (int j=0; j<LIST_MAX; j++)   // Scan the whole key list.
    {
      if ( kpd.key[j].stateChanged )   // Only find keys that have changed state.
      {
        switch (kpd.key[j].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        case PRESSED:
          //  swOnPassKeyPress(kpd.key[i].kchar);
          // swOnQuestionKeyPress(i,kpd.key[j].kchar);
          (*swOnKeyPress)(i,kpd.key[i].kchar);
        break;
        }
      }
    }
  }
}

void checkQuestion(int& i){
  if(keypadReadAnswear.length()<=0){
    lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, delay2K);
    keypadReadAnswear = "";
    
    // lcd2LineMsg("Give the pass...","Password:",0,0,0,1,0,0);
    isEnterActive = false;
    --i;
  }else {
    // ckKeypadAns(i,keypadReadAnswear,lowerBounds[i],upperBounds[i],lcdTypingSentences[i]);
    // if(i==0){
      if(i==3){
        ckPlantRate(i);
      }else{
        ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
        
      }
    //   if(lcdAnswearsArray[i].length() > 0){actualFieldLength = lcdAnswearsArray[i].toInt() - 22;} ;
    // // }else if(i==1){
    //   ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
    //   if(lcdAnswearsArray[i].length() > 0){ckCalcDist(i);} ;
    // // }else if(i==2 || i==4){
    //   ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
    // // }else if(i==3){
    //   ckPlantRate(i);
    // }
    // if(){
    //   check bounds
      
    //  --i;
    // }
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
  lcd.begin(16, 2);                     // initialize the dimensions of display
  // keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
}
void questionMsg(int& i){
  isEnterActive = false;
  // keypad.buff[0] = i; // usefull to sync the i with the eventListener func
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

  // lcdAnswearsArray[i] = (keypadReadAnswear == "#") ? "Yes" : "No";
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
      // --i;
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
      // --i;
    }
    --i;
    keypadReadAnswear = "";
    lcd.clear();
  } // end of serialAns.toInt()<firstBound || serialAns.toInt()>secondBound
  else
  { // serialAns bounds OK!
    //save the answear to the array
    lcdAnswearsArray[i] = keypadAns;
    keypadReadAnswear = "";
    if(i==0){
      // Serial.print(lcdAnswearsArray[i]);
      /* if(lcdAnswearsArray[i].length() <= 0){ */actualFieldLength = lcdAnswearsArray[i].toInt() - 22;/* } ; */
    }else if(i==1){
      /* if(lcdAnswearsArray[i].length() <= 0){ */ckCalcDist(i);/* } ; */
    }else if(i==3){
      ckPlantRate(i);
    }else{}
  } // end of serialAns bounds OK!
}

// bool isNumb(KeypadEvent key)
// {
//   for (byte i(0); i < ROWS; i++)
//   {
//     for (byte j(0); j < COLS - 1; j++)
//     {
//       if (keys[i][j] == key)
//       {
//         return true;
//       }
//     }
//   }
//   return false;
// };
// bool isAlpha(KeypadEvent key)
// {
//   for (byte i(0); i < ROWS; i++)
//   {
//     for (byte j(COLS - 1); j < COLS; j++)
//     {
//       if (keys[i][j] == key)
//       {
//         return true;
//       }
//     }
//   }
//   return false;
// };
// void keypadEvent(KeypadEvent key)
// {
//   state = keypad.getState();
//   switch (state)
//   {
//   case PRESSED:
//     if (isNumb(key))
//     {
//       if (key != '*' && key != '#')
//       {
//         switch (keypad.buff[0])
//         {
//         case 0:
//           if (keypadReadAnswear.length() < String(upperBounds[keypad.buff[0]]).length())
//           { //5 chars max cause length upperbound is 25000
//              concatKeysAndPrint(key,CONTAINS_CM);//concat the key to keypadReadAnswear
//           }
//           else
//           {
//              warnTooManyChars("Too many digits...",CONTAINS_CM);// promps msg if u write more that 5 chars
//           }
//           break;
//         case 1:
//           if (keypadReadAnswear.length() < String(upperBounds[keypad.buff[0]]).length())
//           { //5 chars max cause width upperbound is 12500
//             concatKeysAndPrint(key,CONTAINS_CM);//concat the key to keypadReadAnswear
//           }
//           else
//           {
//             warnTooManyChars("Too many digits...",CONTAINS_CM);// promps msg if u write more that 5 chars
//           }
//           break;
//         case 2:
//           if (keypadReadAnswear.length() < String(upperBounds[keypad.buff[0]]).length())
//           { //1 chars max cause numb of trees uppe is 6
//             concatKeysAndPrint(key);//concat the key to keypadReadAnswear
//           }
//           else
//           {
//             warnTooManyChars("Too many digits...");// promps msg if u write more that 5 chars
//           }
//           break;
//         case 3:
//           break;
//         case 4:
//           if (keypadReadAnswear.length() < String(plantRate).length())
//           { 
//             concatKeysAndPrint(key);//concat the key to keypadReadAnswear
//           }
//           else
//           {
//             warnTooManyChars("Too many digits...");
//           }
//           break;
//         }
//       }
//       else if (key == '*')
//       {
//         if (keypad.buff[0] == 3)
//         {
//           if (keypadReadAnswear.length() < 1)
//           {
//             keypadReadAnswear += key;
//             lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], (keypadReadAnswear == "*") ? "No" : "", 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
//           }
//           else
//           {
//             lcd2LineMsg("too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
//             lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], (keypadReadAnswear == "*") ? "No" : "Yes", 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
//           }
//         }
//       }
//       else
//       { // key == '#'
//         if (keypad.buff[0] == 3)
//         {
//           if (keypadReadAnswear.length() < 1)
//           {
//             keypadReadAnswear += key;
//             lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], (keypadReadAnswear == "#") ? "Yes" : "", 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
//           }
//           else
//           {
//             lcd2LineMsg("too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
//             lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], (keypadReadAnswear == "#") ? "Yes" : "No", 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
//           }
//         }
//       }
//     }
//     else if (isAlpha(key))
//     {

//       switch (key)
//       {
//       case 'A':
//         isEnterActive = true;
//         break;
//       case 'B':
//         break;
//       case 'C':
//         if (keypadReadAnswear.length() > 0)
//         { //5 chars max
//           keypadReadAnswear = keypadReadAnswear.substring(0, keypadReadAnswear.length() - 1);
//           if (keypad.buff[0] == 0 || keypad.buff[0] == 1)
//           {
//             lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], (keypadReadAnswear.length() > 0) ? (keypadReadAnswear + "cm") : keypadReadAnswear, 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
//           }
//           else if (keypad.buff[0] == 2 || keypad.buff[0] == 4)
//           {
//             lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], (keypadReadAnswear.length() > 0) ? keypadReadAnswear : keypadReadAnswear, 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
//           }
//           else if (keypad.buff[0] == 3)
//           {
//             String tmporar = "";
//             if (keypadReadAnswear == "#")
//             {
//               tmporar = "Yes";
//             }
//             if (keypadReadAnswear == "*")
//             {
//               tmporar = "No";
//             }
//             lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], (keypadReadAnswear.length() > 0) ? tmporar : tmporar, 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
//           }
//           else
//           { // keypad.buff[0] == 4
//           }
//         }
//         break;
//       case 'D':
//         lcd1LineMsg("Reset...", 0, 0, delay1_5K, LCD_CLEAR);
//         resetFunc();
//         break;
//       default: //In case smth goes to this option catch it
//         break;
//       }
//     }
//     break;
//   case RELEASED: // these are useless right now
//     break;
//   case HOLD:
//     break;
//   case IDLE:
//     break;
//   }
// }


// void checkAnswears(int& i){
//   if(i==0){
//     ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
//     if(lcdAnswearsArray[0].length() > 0){actualFieldLength = lcdAnswearsArray[i].toInt() - 22;} ;
//   }else if(i==1){
//     ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
//     if(lcdAnswearsArray[1].length() > 0){ckCalcDist(i);} ;
//   }else if(i==2 || i==4){
//     ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
//   }else if(i==3){
//     ckPlantRate(i);
//   }
// }

// void warnTooManyChars(String msg,bool containsCm = NO_CM, uint delay = delay1_5K){
//   String cm = (containsCm)? "cm" : "";
//   lcd2LineMsg(msg, "Press C to clear", 0, 0, 0, 1, 0, delay);
//   lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], keypadReadAnswear + cm, 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
// }

// void concatKeysAndPrint(char key,bool containsCm = NO_CM){
//   String cm = (containsCm)?"cm":"" ;
//   keypadReadAnswear += key;
//   lcd2LineMsg(lcdTypingSentences[keypad.buff[0]], keypadReadAnswear + cm, 0, 0, lcdTypingSentences[keypad.buff[0]].length() - 1, 0, 0, 10);
  
// }