/* @MasterMega.ino
||  This is the MasterMega.ino. In this file you can find code based on lcd, keypad, Servo and 
||  Adafruit Motor shield  libraries.
||  It is developed and maintained by Petros Chatzipetrou. Contributors are Sgouros Xristos and Nikos Kagasidis
||  It is a project in frames of the course TELO_3.
||  It is started on 18.12.2017
*/

/*-----( Import needed libraries )-----*/
#include <Wire.h>              // library for intercomunication of arduinos via sda and scl ports
#include <LiquidCrystal_I2C.h> // the lcd libray
#include <Keypad.h>            // the keypad library
#include <Servo.h>             // Servo motors library is used for arm system and driller
#include <AFMotor.h>           //library to move the wheel dc motors

/*----------( END_OF_IMPORT_LIBRARIES ) ----------*/

/*-----( #defines and utiliities )-----*/
typedef unsigned int uint; // create a shortcut of the specified type
#define LCD_CLEAR 1        // usefull for 1lineMsg if u want to use lcd.clear cmd or not
#define LCD_NO_CLEAR 0     // and this one if you dont want to use lcd.cler cmd
#define delay2K 2000       // 2k delay on a variety of functions
#define delay1K 1000       // 1k delay on a variety of functions
#define delay1_5K 1500     // 1.5k delay on a variety of functions
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
#define plantRateLB 6    // plantRate lower bound
#define calcDistUB 25000 // uper bound of calculate distance
#define passMaxChar 7    // max characters that user can fill up on keypad as password
/*----------( END_OF_#DEFINES_AND_UTILITIES_DECLARATIONS ) ----------*/

/*-----( Declare Constants )-----*/
const byte ROWS(4);            // number of Rows of keypad
const byte COLS(4);            // nuber of Columns of keypad
const byte numbOfQuestions(5); // number of the array in lcdQuestionsArray
const byte numbOfAnswears(5);  // number of the array in lcdAnswearsArray
/*----------( END_OF_CONSTANTS_DECLARATIONS ) ----------*/

/*-----( Declare Variables )-----*/
/*
   keypad & screen variables
*/
char keys[ROWS][COLS] = { // overview of the keypad keys that we need for object declaration
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {32, 33, 34, 35};        //connect to the row pinouts of the keypad
byte colPins[COLS] = {36, 37, 38, 39};        //connect to the column pinouts of the keypad
String lcdQuestionsArray[numbOfQuestions] = { // overview of Questions displayed to the User
    "Give the desired Length",
    "Give the desired Width",
    "Give the desired number of trees",
    "Manually _plant_ rate? Y(#)/N(*)",
    "Give the desired plant rate"};
String lcdAnswearsArray[numbOfAnswears] = {""};    // here we will keep the answears of User
String keypadReadAnswear = "";                     // initialise the var that keeps temporar answear of User
String tmpPass = "";                               // used to turn the numbers that user press on keypad to asterisk (*)
String userPassAns = "";                           //temporar variable for storing user answear from keypad
String passwords[] = {"8888", "0000"};             //1111 User Mode , 0000 Maintenance Mode
String mode[] = {"User mode", "Maintenance mode"}; //used in checkModeAndPrintMsg, based on user answear of password
bool isEnterActive(false);                         // check if enter is pressed
String lcdTypingSentences[numbOfQuestions] = {     //used in some functions related to the question above
    "Length:", "Width:", "Num of trees:", "Your Answear:", "Pl. rate:"};
uint lowerBounds[numbOfQuestions] = {lengthLB, widthLB, numbOfTreesLB, 1, plantRateLB};
uint upperBounds[numbOfQuestions] = {lengthUB, widthUB, numbOfTreesUB, 1, 0}; //the last 0 value is calculated on the way
//the desired outcome based on user answears
uint totalRounds(0);       //Number of courses in Specifications 2.2.5
uint actualFieldLength(0); //Length of courses in Specifications 2.2.5
uint plantRate(0);         // the manually or automatic plant rate based on user answears
/*
  Hygrometer sensor variables 
*/
const int hydroSensorPin = A14; // hydro pin number on arduino mega
byte hydroValuePercent(0);      // used to represent the values of hydrometer (0-1023) in percentage(0-100%)
/*
  Led photoresistor variables
*/
const byte ledL = 42; //led pin numbers on arduino mega
const byte ledR = 43;
const int photoresistorPin = A13; // the analog pin number of the photoresistor
/* 
  HAll sensor variables, first is Left Hall second is Right Hall
*/
String first = "";     // used in hallDistCalc to coppy the data from the transmission of the left hall encoder
String second = "";    // this is for right hall encoder
int leftHallDist = 0;  // this is the integer representation of variable first
int rightHallDist = 0; // this is the integer representation of variable second
/* 
    arm variables - the beneath variables are a contribution by Xristos Sgouros
*/
byte BSpos = 140; // Starting position for Body
byte SSpos = 45;  // Starting position for Shoulder
byte ESpos = 50;  // Starting position for Elbow
byte GOpos = 75;  // Open Grapple position
byte GCpos = 120; // Close Grapple position
byte Bpos = 0;
byte Spos = 0;
byte Epos = 0;
byte Gpos = 0;

/*----------( END_OF_VARIABLE_DECLARATIONS ) ----------*/

/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 16 chars 2 line display
// LCD object         addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// keypad object
// check API at https://playground.arduino.cc/Code/Keypad#Download
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
//The left and right motor object
AF_DCMotor motorL(1); // left motor on motorShield 1 slot
AF_DCMotor motorR(2); //right motor on motorShield 2 slot
// servo motors objects to controll the arm
//the beneath declarations are a contribution from Sgouros Xristos
Servo Body;     // create servo object control of the lower servo motor
Servo Shoulder; // create servo object control of the right servo motor
Servo Elbow;    // create servo object control of the left servo motor
Servo Grapple;  // create servo object control of the upper servo motor

/*----------( END_OF_OBJECT_DECLARATIONS ) ----------*/

/*-----( Declare Functions )-----*/
void (*resetFunc)(void) = 0; //this is the software reset that looks at addr 0x00
//write to screen 1 line of message(message, cursot at position,cursor at line,how long to be displayed,clear lcd before run this func or no)
void lcd1LineMsg(String msg, byte cursorCharAt, byte cursorLineAt, uint Delay, bool lcdClear);
//write to screen 2 line of messages, it's based on lcd1LineMsg func almost same arguments
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, uint msg1Delay, uint msg2Delay);
//check the users answear about the bounds of length width plant rate etc..
void ckKeypadAns(int &i, String keypadAns, uint firstBound, uint secondBound, String boundsAns);
void ckPlantRate(int &i);                                         // handle plant rate question based on user's response.Manual or Auto?
void ckCalcDist(int &i);                                          //calculates totalRounds and if it is out of bounds it prints msg and reset..
void questionMsg(int &i);                                         // prints each time the question that are stored on lcdQuestionsArray
void init0();                                                     // simple initialization like serial.begin, wire begin, lcd.begin etc..
void checkAndPrintQuestion(int i, char key);                      // checks the user's answear based on the question of the system and prints them out . also informs user when he writes too many chars
void swOnQuestionKeyPress(int i, char key);                       // switch on every keypad key press and take actions
void checkQuestion(int &i);                                       //checks if user pushed a button as answear  and then runs ckKeypadAns func
void checkAndPrintPass(byte Passlength, char key);                // do the same work as checkAndPrintQuestion but based on password rules this time
void swOnPassKeyPress(char key);                                  // the same rules as swOnQuestionKeyPress but for password actions
void checkPassword();                                             //the same as checkQuestion and then checks if user put the correct passwords
void checkModeAndPrintMsg();                                      // check mode based on user's pasword and print apropriate msg to screen
void handlePassAndPrintMsg();                                     // check the user's password and print apropriate msg to screen
void printQAndCkUserResponse();                                   //prints the questions and check  the user answear to be between bounds and continue
void finalMsgAndCLoseScreen();                                    // prints the final msg to user about the plant rate and the lets start.. then after a while closes the screen
byte cnvHydroValToPercent(int sensorValOfHydro);                  // This function is a contribution from Nikos Kagasidis. It converts the the sensor value of Hydrometer to percentage
void checkHydroValue();                                           //checks for hydrometer values and if it's Greater than 50 continues else prints warning msg
void ckLightsOfCar();                                             // checks if there is a need to turn the lights on or off in the begining
void Plant();                                                     //this function is a contribution from Xristos Sgouros . Used for doing the plant process with the arm. It grabs a tree and put it in the hole using the servo motors
void setMotorSpeedLeftToRight(byte left, byte right);             // sets the motors speed (0-255) first argument controlls th left motor and the second the right one
void moveNplantThroughDesiredFieldLength(uint actualFieldLength); // moves across the desired user input of field length and doing the plant process in paralell
uint cmToHallDist(uint actualFieldLeng);                          // tranpiles the cm from user acttual field length input to Hall encoder pulses
void hallDistCalc();                                              // transfers data from slave hall encoder to the master and then updates the leftHallDist and rightHallDist variables
void stopOrStartEngines(byte val);                                // based on value of argument (0 or 1) starts engines if val = 1 and stops engines if val = 0
/*----------( END_OF_FUNCTION_DECLARATIONS ) ----------*/

void setup() /*----( SETUP: RUNS ONCE )----*/
{
    init0();                   //initialization of screen, pins etc..
    handlePassAndPrintMsg();   // handles the user keypresses and print msg to screen
    checkModeAndPrintMsg();    //chekcs user or maintenance mode and take apropriate actions based on requirments
    checkHydroValue();         // checks Hydro and print msg
    printQAndCkUserResponse(); // start writing the answears on lcd and check user answears
    finalMsgAndCLoseScreen();  //prints the plant rate and lets start msg and then closes the screen
    ckLightsOfCar();           // check photoresistor to turn lights on
    if (plantRate % 2 != 0)    // plant rate should be even numb cause we observerd that hall encoder gives even outcome of pulses(0,2,4,8,12, etc..), so in order to be devided with plantRate val, we make the plantRate val even.
        plantRate += 1;
    }
} /*--(end setup )---*/when we use round()
    {

void loop() /*----( LOOP: RUNS CONSTANTLY )----*/
{
    moveNplantThroughDesiredFieldLength(actualFieldLength);

} /* --(end main loop )-- */

/* ---------- ( Function Definition )----------- */

/* 
@function
@abstract Writes to screen one line message.

@param  msg | The message u want to write on screen.Be carefull, don't exceed 16chars.
@param  cursorCharAt | The place you want to put the curson on the screen (0-16).
@param  cursorLineAt | The line you want to sellect (0-1). 
@param  Delay | The delay you want to keep the message on the screen. By default this values is 0.
@param  lcdClear | If you want to clear the screen before displaying your. By default it doesn't clear the screen.
*/
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

/* 
@function
@abstract Writes to screen two line messages.

@param  firstMsg | The  first line message u want to write on screen.Be carefull, don't exceed 16chars.
@param  secondMsg | The  second line message u want to write on screen.Be carefull, don't exceed 16chars.
@param  msg1CursorCharAt | The place you want to put the curson on the screen (0-16) of the first line message.
@param  msg1CursorLineAt | The first message line you want to sellect (0-1).
@param  msg2CursorCharAt | The place you want to put the curson on the screen (0-16) of the second line message.
@param  msg2CursorLineAt | The second message line you want to sellect (0-1).
@param  msg1Delay | The delay you want to keep the message on the screen of the first message. By default this values is 0.
@param  msg2Delay | The delay you want to keep the message on the screen of the first message. By default this values is 0.
*/
void lcd2LineMsg(String firstMsg, String secondMsg, byte msg1CursorCharAt, byte msg1CursorLineAt, byte msg2CursorCharAt, byte msg2CursorLineAt, uint msg1Delay = 0, uint msg2Delay = 0)
{
    lcd1LineMsg(firstMsg, msg1CursorCharAt, msg1CursorLineAt, msg1Delay, LCD_CLEAR);
    lcd1LineMsg(secondMsg, msg2CursorCharAt, msg2CursorLineAt, msg2Delay);
}

/* 
@function
@abstract   Checks the user answear from keypad.Checks for the bounds of the input values and warning the user in case he writes values out of bounds.
            if everything is ok, saves the user input value to lcdAnswearsArray[] and consequently calculates the actual field length which is the field
            Length  -  22cm (the vehicles length).

@param  &i | keep track of the lcdQuestionsArray[] sequence.
@param  keypadAns | Here we save user input value.
@param  firstBound | This is the lower bound we want to have.
@param  secondBound | This is the upper bound we want to have.
@param  boundsAns | Here we put the asnwear we want to be in between the two bounds we declare.
*/
void ckKeypadAns(int &i, String keypadAns, uint firstBound, uint secondBound, String boundsAns)
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
        }    //end of serialAns.length()<1
        else /* (!tmpAns) */
        {
            String tmp = (boundsAns + ":" + keypadAns);
            lcd2LineMsg(/* (keypad.buff[0] == 2 || keypad.buff[0] == 4) ? */ tmp /* : (tmp + "cm") */, "Out of bounds...", 0, 0, 0, 1, 0, delay2K);
            lcd1LineMsg("Try again...", 0, 0, delay1_5K, LCD_CLEAR);
            // tmp is helpfull to turn the whole sentense that we wanna use into type String
            String lastSentenceToString = (String(firstBound) + "<" + boundsAns + "<" + String(secondBound));
            lcd2LineMsg("Bounds between", lastSentenceToString, 0, 0, 0, 1, 0, delay2K);
        }
        --i;
        keypadReadAnswear = "";
        lcd.clear();
    } // end of serialAns.toInt()<firstBound || serialAns.toInt()>secondBound
    else
    {                                    // serialAns bounds OK!
        lcdAnswearsArray[i] = keypadAns; //save the answear to the array
        keypadReadAnswear = "";
        if (i == 0)
        {
            actualFieldLength = lcdAnswearsArray[i].toInt() - 22;
        }
        else if (i == 1)
        {
            ckCalcDist(i);
        }
        else if (i == 3)
        {
            ckPlantRate(i);
        }
        else
        {
        }
    } // end of serialAns bounds OK!
}

/* 
@function
@abstract   We check based on user inputs if the calculated distance in in bounds. If not we print message and then restart the system.
            If it's in bounds, we print the calculated distance the car is going to run. We also calculate the total rounds variable

@param  &i | keep track of the lcdQuestionsArray[] sequence.
*/
void ckCalcDist(int &i)
{
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

/* 
@function
@abstract   We check based on user inputs if the calculated distance in in bounds. If not we print message and then restart the system.
            If it's in bounds, we print the calculated distance the car is going to run. We also calculate the total rounds variable

@param  &i | keep track of the lcdQuestionsArray[] sequence.
*/
void ckPlantRate(int &i)
{
    float calc = actualFieldLength * totalRounds; //auto calc for plant rate
    plantRate = round(calc / lcdAnswearsArray[i - 1].toInt());
    upperBounds[i + 1] = plantRate; // add these value in case user wants manual plant rate
    keypadReadAnswear = "";         // clean keypad in case user goes to manual plant rate
    if (lcdAnswearsArray[i] == "No")
    {
        lcd1LineMsg("Plant rate:" + (String)plantRate, 0, 0, 0, LCD_CLEAR);
        ++i; //  jump to the next itteration and avoid last question
    }
}

/* 
@function
@abstract   Prints the message based on lcdQuestionsArray[]

@param  &i | keep track of the lcdQuestionsArray[] sequence.
*/
void questionMsg(int &i)
{
    isEnterActive = false;
    String sub1 = lcdQuestionsArray[i].substring(0, 16);
    String sub2 = lcdQuestionsArray[i].substring(16);
    sub2.trim(); //trims the second sentence to avoid white spaces
    // Prints out the questions
    lcd2LineMsg(sub1, sub2, 0, 0, 0, 1, 0, delay2K);
    lcd1LineMsg(lcdTypingSentences[i], 0, 0, 0, LCD_CLEAR);
}

/*
@function
@abstract   initialise pinModes, objects 
*/
void init0()
{
    Serial.begin(115200);  // Used to type in characters to Serial monitor( usefull for debugging)
    Wire.begin();          //begin the transmission between arduinos begin() with empty argument reffers to master
    lcd.begin(16, 2);      // initialize the dimensions of display
    pinMode(ledL, OUTPUT); // declare pinMode for Leds
    pinMode(ledR, OUTPUT);
    digitalWrite(ledL, LOW); //initially lights off
    digitalWrite(ledR, LOW);
    //the beneath is contribution by Xristos Sgouros
    Body.attach(22);     //attaches the servo on pin 22
    Shoulder.attach(24); //attaches the servo on pin 24
    Elbow.attach(26);    //attaches the servo on pin 26
    Grapple.attach(28);  //attaches the servo on pin 28
    Body.write(140);
    Shoulder.write(SSpos);
    Elbow.write(ESpos);
    Grapple.write(GOpos);
}

/*
@function
@abstract   Cocatenates the key presses and check if the answear exceeds the maximum characters based on its bounds.
            It prints user iput each time of a key press.

@param  i |  keep track of the lcdQuestionsArray[] sequence.
@param  key | put the key presses of keypad.
*/
void checkAndPrintQuestion(int i, char key)
{
    if (keypadReadAnswear.length() < String(upperBounds[i]).length())
    {
        keypadReadAnswear += key;
        lcd1LineMsg((lcdTypingSentences[i] + keypadReadAnswear), 0, 0, 0, LCD_CLEAR);
    }
    else
    {
        lcd2LineMsg("Too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
        lcd1LineMsg((lcdTypingSentences[i] + keypadReadAnswear), 0, 0, 0, LCD_CLEAR);
    }
}

/*
@function
@abstract   switch on  key press. Checks if the character is digit, letter, asterisk(*) or sharp(#) and calls checkAndPrintQuestion

@param  i |  keep track of the lcdQuestionsArray[] sequence.
@param  key | put the key presses of keypad.
*/
void swOnQuestionKeyPress(int i, char key)
{
    if (isDigit(key))
    {
        if (i != 3)
        {
            checkAndPrintQuestion(i, key);
        }
    }
    else if (isAlpha(key))
    {
        switch (key)
        {
        case 'A':
            isEnterActive = true;
            break;
        case 'C':
            if (keypadReadAnswear.length() > 0)
            {
                keypadReadAnswear = keypadReadAnswear.substring(0, keypadReadAnswear.length() - 1);
                lcd1LineMsg((lcdTypingSentences[i] + keypadReadAnswear), 0, 0, 0, LCD_CLEAR);
            }
            break;
        case 'D':
            lcd1LineMsg("Reset...", 0, 0, delay1_5K, LCD_CLEAR);
            resetFunc();
            break;
        }
    }
    else
    {
        if (i == 3)
        {
            if (keypadReadAnswear.length() < String(upperBounds[i]).length())
            {
                keypadReadAnswear += key;
                if (keypadReadAnswear == "#")
                {
                    lcdAnswearsArray[i] = "Yes";
                }
                else
                {
                    lcdAnswearsArray[i] = "No";
                }
                lcd1LineMsg((lcdTypingSentences[i] + lcdAnswearsArray[i]), 0, 0, 0, LCD_CLEAR);
            }
            else
            {
                lcd2LineMsg("Too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
                lcd1LineMsg((lcdTypingSentences[i] + lcdAnswearsArray[i]), 0, 0, 0, LCD_CLEAR);
            }
        }
    }
}


/*
@function
@abstract   check if user answear is empty or not.If it is empty it means he pressed enter button(A). If  not, calls ckKeypadAns or ckPlantRate if param i is equal to 3

@param  i |  keep track of the lcdQuestionsArray[] sequence.
*/
void checkQuestion(int &i)
{
    if (keypadReadAnswear.length() <= 0)
    {
        lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, delay2K);
        keypadReadAnswear = "";
        isEnterActive = false;
        --i;
    }
    else
    {
        if (i == 3)
        {
            ckPlantRate(i);
        }
        else
        {
            ckKeypadAns(i, keypadReadAnswear, lowerBounds[i], upperBounds[i], lcdTypingSentences[i]);
        }
    }
}

/*
@function
@abstract   checks the password length to not exceed the maximum characters. Also print the password on screen in the form of asterisks(*) to obscure the real one.

@param  Passlength |  the password maximum length you want to have
@param  key | put the key presses of keypad.
*/
void checkAndPrintPass(byte Passlength, char key)
{
    if (keypadReadAnswear.length() < Passlength)
    {
        tmpPass = "";
        keypadReadAnswear += key;
        for (int i = 0; i < keypadReadAnswear.length(); i++)
        {
            tmpPass += "*";
        }
        lcd2LineMsg("Give the pass...", ("Password:" + tmpPass), 0, 0, 0, 1, 0, 0);
    }
    else
    {
        lcd2LineMsg("Too many chars...", "Press C to clear", 0, 0, 0, 1, 0, 1500);
        lcd2LineMsg("Give the pass...", ("Password:" + tmpPass), 0, 0, 0, 1, 0, 0);
    }
}

/*
@function
@abstract   switch on  key press. Checks if the character is digit, letter, asterisk(*) or sharp(#) and calls checkAndPrintPass

@param  key | put the key presses of keypad.
*/
void swOnPassKeyPress(char key)
{
    if (isDigit(key))
    {
        checkAndPrintPass(passMaxChar, key);
    }
    else if (isAlpha(key))
    {
        switch (key)
        {
        case 'A':
            isEnterActive = true;
            break;
        case 'C':
            if (keypadReadAnswear.length() > 0)
            {
                tmpPass = "";
                keypadReadAnswear = keypadReadAnswear.substring(0, keypadReadAnswear.length() - 1);
                for (int i = 0; i < keypadReadAnswear.length(); i++)
                {
                    tmpPass += "*";
                }
                lcd2LineMsg("Give the pass...", ("Password:" + tmpPass), 0, 0, 0, 1, 0, 0);
            }
            break;
        case 'D':
            lcd1LineMsg("Reset...", 0, 0, delay1_5K, LCD_CLEAR);
            resetFunc();
            break;
        }
    }
    else
    {
        checkAndPrintPass(passMaxChar, key);
    }
}

/*
@function
@abstract   checks if the the password entered is valid (user or maintenance mode). And informs the user with apropriate message.
*/
void checkPassword()
{
    if (keypadReadAnswear.length() <= 0)
    {
        lcd2LineMsg("You must write", "a value...", 0, 0, 0, 1, 0, delay2K);
        keypadReadAnswear = "";
        tmpPass = "";
        lcd2LineMsg("Give the pass...", "Password:", 0, 0, 0, 1, 0, 0);
        isEnterActive = false;
    }
    else
    {
        if (keypadReadAnswear != passwords[0] && keypadReadAnswear != passwords[1])
        {
            isEnterActive = false;
            lcd1LineMsg("Wrong Pass...", 0, 0, delay1_5K, LCD_CLEAR);
            lcd1LineMsg("Try again...", 0, 0, delay1_5K, LCD_CLEAR);
            keypadReadAnswear = "";
            tmpPass = "";
            lcd2LineMsg("Give the pass...", "Password:", 0, 0, 0, 1, 0, 0);
        }
    }
}

/*
@function
@abstract   Prints an apropriate message based on the chosen mode(user or maintenance).
*/
void checkModeAndPrintMsg()
{
    lcd1LineMsg("Bingo!", 0, 0, 0, LCD_CLEAR);
    userPassAns = keypadReadAnswear;
    delay(delay1K);
    for (int i = 0; i < (sizeof(passwords) / sizeof(passwords[0])); i++)
    {
        if (userPassAns == passwords[i])
        {
            lcd2LineMsg("Welcome to", mode[i], 0, 0, 0, 1, 0, delay2K);
        }
    }
    keypadReadAnswear = "";
    if (userPassAns == passwords[1])
    {
        //TODO move all motors
        while (1)
        {
        }
    }
}

/*
@function
@abstract   prints give the password message and the user answear each time he pressed a key. if he press enter(A), it calls checkPassword function.
*/
void handlePassAndPrintMsg()
{
    lcd2LineMsg("Give the pass...", "Password:", 0, 0, 0, 1, 0, 0);
    while (keypadReadAnswear != passwords[0] && keypadReadAnswear != passwords[1])
    {
        while (!isEnterActive)
        {
            if (kpd.getKeys())
            {
                for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
                {
                    if (kpd.key[i].stateChanged) // Only find keys that have changed state.
                    {
                        switch (kpd.key[i].kstate)
                        { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
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

/*
@function
@abstract   prints the Questions of the lcdQuestionsArray[] and writes the user inputs on screen based on each question.If enter button is pressed(A), checkQuestion function is called.
*/
void printQAndCkUserResponse()
{
    for (int i = 0; i < (sizeof(lcdQuestionsArray) / sizeof(lcdQuestionsArray[0])); i++)
    {
        questionMsg(i);
        while (!isEnterActive)
        {
            if (kpd.getKeys())
            {
                for (int j = 0; j < LIST_MAX; j++) // Scan the whole key list.
                {
                    if (kpd.key[j].stateChanged) // Only find keys that have changed state.
                    {
                        switch (kpd.key[j].kstate)
                        { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                        case PRESSED:
                            swOnQuestionKeyPress(i, kpd.key[j].kchar);
                            break;
                        }
                    }
                }
            }
        }
        checkQuestion(i);
    }
}

/*
@function
@abstract   prints Let's start message and closes the backlight and the monitor of the screen to save battery.
*/
void finalMsgAndCLoseScreen()
{
    lcd1LineMsg("Let's start...", 0, 1, delay2K * 2, LCD_NO_CLEAR);
    lcd.clear();
    lcd.off();
}

/*
@function
@abstract   This function is a contibution from Nikos Kagasidis. It converts the hydrometer analog values(0-1023) to percentage

@param  sensorValOfHydro | Here we put the analog value of the hydrometer.
*/
byte cnvHydroValToPercent(int sensorValOfHydro)
{
    byte percVal = map(sensorValOfHydro, 430, 1023, 100, 0);
    return percVal;
}

/*
@function
@abstract   checks the percentage of the hydrometer and if it is < 50%, prints message about the low humidity.if it's > 50% prints Bingo! letting the system to go on.
*/
void checkHydroValue()
{
    hydroValuePercent = cnvHydroValToPercent(analogRead(hydroSensorPin));
    if (hydroValuePercent >= 50)
    {
        lcd1LineMsg(("humidity is:" + String(hydroValuePercent) + "%"), 0, 0, delay1K, LCD_CLEAR);
    }
    while (hydroValuePercent < 50)
    {
        lcd1LineMsg(("humidity is:" + String(hydroValuePercent) + "%"), 0, 0, delay2K, LCD_CLEAR);
        lcd2LineMsg("humidity is low", "Water the ground", 0, 0, 0, 1, 0, delay1K);
        hydroValuePercent = cnvHydroValToPercent(analogRead(hydroSensorPin));
    }
    lcd2LineMsg("Bingo!!!", ("humidity is:" + String(hydroValuePercent) + "%"), 0, 0, 0, 1, 0, delay2K);
}

/*
@function
@abstract   checks the photoresistor values. if it's <= 50, opens the lights of vehicle otherwise it's off.
*/
void ckLightsOfCar()
{
    if (analogRead(photoresistorPin) <= 50)
    {
        digitalWrite(ledL, HIGH);
        digitalWrite(ledR, HIGH);
    }
}

/*
@function
@abstract   This functions is a contribution from Sgouros Xristos. It moves the arm motors, letting the system pick a tree from the warehouse and put it in the hole.  
*/
void Plant()
{
    //--------------Balance Position (A0)-------------------//               go to balance position
    Body.write(140);
    Shoulder.write(SSpos);
    Elbow.write(ESpos);
    Grapple.write(GOpos);
    delay(1000);
    //--------------Go to tree Position(A1)------------------//
    for (Bpos = 140; Bpos >= 70; Bpos--)
    {
        Body.write(Bpos);
        delay(30);
    }
    delay(500);
    for (Epos = ESpos; Epos <= 90; Epos++)
    {
        Elbow.write(Epos);
        delay(30);
    }

    delay(500);
    for (Spos = SSpos; Spos >= 35; Spos--)
    {
        Shoulder.write(Spos);
        delay(30);
    }
    delay(500);
    for (Epos = 90; Epos <= 100; Epos++)
    {
        Elbow.write(Epos);
        delay(30);
    }
    delay(500);
    for (Spos = 35; Spos >= 30; Spos--)
    {
        Shoulder.write(Spos);
        delay(30);
    }
    delay(500);
    for (Bpos = 70; Bpos >= 9; Bpos--)
    {
        Body.write(Bpos);
        delay(30);
    }
    delay(500);
    for (Spos = 35; Spos <= 44; Spos++)
    {
        Shoulder.write(Spos);
        delay(30);
    }
    delay(500);
    for (Gpos = GOpos; Gpos <= GCpos; Gpos++)
    {
        Grapple.write(Gpos);
        delay(0);
    }
    delay(500);
    //--------------Go to Plant Position(A2)------------------//
    for (Epos = 101; Epos <= 110; Epos++)
    {
        Elbow.write(Epos);
        delay(30);
    }
    delay(500);
    for (Bpos = 9; Bpos <= 165; Bpos++)
    {
        Body.write(Bpos);
        delay(30);
    }
    delay(500);
    for (Spos = 44; Spos <= 90; Spos++)
    {
        Shoulder.write(Spos);
        delay(30);
    }
    delay(500);
    for (Epos = 110; Epos >= 70; Epos--)
    {
        Elbow.write(Epos);
        delay(30);
    }
    delay(500);
    for (Spos = 90; Spos <= 150; Spos++)
    {
        Shoulder.write(Spos);
        delay(30);
    }
    delay(500);
    for (Epos = 70; Epos >= 10; Epos--)
    {
        Elbow.write(Epos);
        delay(30);
    }
    delay(500);
    for (Gpos = GCpos; Gpos >= GOpos; Gpos--)
    {
        Grapple.write(Gpos);
        delay(0);
    }
    delay(500);
    //--------------Go to Balance Position Position(A0)------------------//
    for (Spos = 150; Spos >= 80; Spos--)
    {
        Shoulder.write(Spos);
        delay(30);
    }
    delay(500);
    for (Epos = 10; Epos <= ESpos; Epos++)
    {
        Elbow.write(Epos);
        delay(30);
    }
    delay(500);
    for (Bpos = 165; Bpos >= BSpos; Bpos--)
    {
        Body.write(Bpos);
        delay(30);
    }
    delay(500);
    for (Spos = 80; Spos >= SSpos; Spos--)
    {
        Shoulder.write(Spos);
        delay(30);
    }
    delay(500);
}

/*
@function
@abstract   Sets the motors' speed and run forward.

@param  left | the left motor set speed.
@param  right | the right motor set speed.
*/
void setMotorSpeedLeftToRight(byte left, byte right)
{
    motorR.setSpeed(right);
    motorL.setSpeed(left);
    motorR.run(FORWARD);
    motorL.run(FORWARD);
}

/*
@function
@abstract   Here we put the actualFieldLength variable and  tranlate its centimeter value to Hall encoder pulses. Then we comunicate with the slave arduino requesting its Hall encoder values
            if the hall encoder measurements doesn't reach the actualFieldLength value, runs the vehivle forward. While running checks for the plantRate variable
            and stops the vehicle based on it to do the plant process. After reached the actualFieldLength value, stops the engines.

@param  actFieldLength | we put the actualFieldLength variable value.
*/
void moveNplantThroughDesiredFieldLength(uint actFieldLength)
{
    uint actualFieldDist = cmToHallDist(actFieldLength);
    while (rightHallDist <= actualFieldDist)
    {
        setMotorSpeedLeftToRight(235, 235);
        Wire.requestFrom(5, 6, false); // request 6 bytes from slave device #5
        hallDistCalc();                //calculate hall left and right  distance
        if (rightHallDist % plantRate == 0 && rightHallDist >= plantRate)
        {
            setMotorSpeedLeftToRight(0, 0);
            Plant();
        }
    }
    setMotorSpeedLeftToRight(0, 0);
}

/*
@function
@abstract   converts  centimeters to pulses.   

@param  actualFieldLeng | we put the actualFieldLength variable to convert it in pulses.
*/
uint cmToHallDist(uint actualFieldLeng)
{
    float tmp = ((float)actualFieldLeng / 0.05314); //1 pulse = 0.05314cm
    uint cm = round(tmp);
    return cm;
}

/*
@function
@abstract   read from the slave arduino via wire comunication the data that the slave sends and save them to rightHallDist variable.
*/
void hallDistCalc()
{
    first = "";
    second = "";
    while (Wire.available()) // slave may send less than requested
    {

        char c = Wire.read();
        if (c != ',')
        {
            second += c;
        }
        else
        {
            while (Wire.available())
            {
                c = Wire.read();
                first += c;
            }
        }
    }
    byte cnt = 0;
    for (int i = 0; i < second.length(); i++)
    {
        if (!isDigit(second[i]))
        {
            cnt++;
        }
    }
    second = second.substring(0, second.length() - cnt);
    rightHallDist = second.toInt();
    //   rightHallDist = duration;
    // cnt = 0;
    // for(int i = 0; i<second.length();i++){
    //   if(!isDigit(second[i])){
    //     cnt++;
    //   }
    // }
    // second = second.substring(0,second.length()-cnt);
    // rightHallDist = second.toInt();
}

/*
@function
@abstract   stops or starts the engines based on parameter. if the parameter is 0, it opens a comunication with the slave arduino which has the motors attached to it and stop them. if it's 1, the engines start.

@param  val | values between 0-1. 0 stops engines 1 starts engines.
*/
void stopOrStartEngines(byte val)
{
    if (val == 0)
    {
        Wire.beginTransmission(10); // transmit to device #10
        Wire.write(val);            // sends one byte
        Wire.endTransmission();
    }
    else if (val == 1)
    {
        Wire.beginTransmission(10); // transmit to device #10
        Wire.write(val);            // sends one byte
        Wire.endTransmission();
    }
    else
    {
    }
}
