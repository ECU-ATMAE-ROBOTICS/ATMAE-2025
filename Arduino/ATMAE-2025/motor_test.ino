//ssh ubuntu@192.168.0.157
// 192.168.0.228
//port 9000

/*
Pin 5
open 0
close at 90

Pin 4
open at 180 
close at 90



*/

// #define LED_PIN 11

#include <Servo.h>

//---PINS---

const int rightDriveMotorPin = 10;
const int leftDriveMotorPin = 13;

const int screwServoPin = 44;
const int topLiftLimitSwitch = 52;
const int bottomLiftLimitSwitch = 39;

const int leftPaddlePin = 11;
const int rightPaddlePin = 9;
const int clampPin = 12;

const int topAirLockPin = 2;
const int bottomAirLockPin = 3;

const int colorSortPin = 6;
const int limitSwitchPinOne = 37;
const int limitSwitchPinTwo = 31;
const int limitSwitchPinThree = 33;
const int limitSwitchPinFour = 35;

Servo rightPaddle;
Servo leftPaddle;

// ID's of controller buttons
const int TELEOP_ID = 22;
const int AUTO_ID = 21;
const int NEUTRAL_ID = 23;
const int LEFT_TRIGGER_ID = 9;
const int RIGHT_TRIGGER_ID = 10;
const int LEFT_STICK_ID = 5;
const int Right_STICK_IDY = 8;
const int Right_STICK_IDX = 7;

const int RDPAD = 2;
const int LDPAD = 4;

//Instruction ID and Value received from the Pi
String receivedData = "";
int button_id;
double axis_val;

//Value for determining robot speed
double RightTrigger = 0;
double LeftTrigger = 0;
double LeftStick = 0;

//Ratio to slow each motor for turning while driving
double leftTurn = 0;
double rightTurn = 0;
double drive = 0;

//PWM values passed to the motors
double LMotor = 1500;
double RMotor = 1500;

bool Auto = false;

bool clampOpen = false;

bool screwStopped = true;



//Motor Servo
Servo leftservo;
Servo rightservo;

//clamp code
Servo clamp;


// Defines  the Smart Servo Sorting that move the color sorter
Servo colorSort;


//placeholder values for color pos changed later
int sortSpeed = 70;
int redPin = 30;
int yellowPin = 60;
int greenPin = 120;
int bluePin = 150;


//limit Switchs and Servo for the Lifting System
Servo screw;




//Pins for each servo gate
const int leftPipeGate = 5;
const int midPipeGate = 7;
const int rightPipeGate = 4;
const int sepPipeGate=8;
//Gate Servos
Servo rightPipeGateServo;
Servo midPipeGateServo;
Servo leftPipeGateServo;
Servo sepPipeGateServo;


//Airlock Servos
Servo topAirLock;
Servo bottomAirLock;

//Positions for servos to open and close
const int openGatePosition = 180;
const int closeGatePosition = 90;

const int openTopAirLockPosition = 90;
const int openLowerAirLockPosition = 180;

const int closeTopAirLockPosition = 180;
const int closeLowerAirLockPosition = 90;

double RightStick = 0;

bool screwOverRide = false;  //Used to control screw during teleop

// Variable to hold the color being sorted
int currentPin = 1;
String colorPins[4];
bool reachedTop = true;
bool reachedBottom = true;
bool innitPosRed = false;
bool innitPosYel = false;
/*
----upperAirLock----
Pin = 2
Close = 180
Open = 90

---lowerAirLock----
Pin = 3
Close = 90
Open = 180

---leftPipeGate---
Pin = 5
Close = 0
Open = 90

---midPipeGate---
Pin = 7
Close = 90
Open = 180

---rightPipeGate---
Pin = 4
Close = 90
Open = 180
*/


/*
Parses instructions from the PI
to determine robot movement
*/
void parseData(String data);

/*
Runs at the start Attach to pins and servo in Startings Pos
*/
void setup() {

  Serial.begin(9600);
  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, HIGH);
  // colorSort.write(sortSpeed);

  //Initialize the servos
  leftservo.attach(leftDriveMotorPin);
  rightservo.attach(rightDriveMotorPin);
  colorSort.attach(colorSortPin);
  clamp.attach(clampPin);
  topAirLock.attach(topAirLockPin);
  bottomAirLock.attach(bottomAirLockPin);
  //leftPaddle.attach(leftPaddlePin);
  //rightPaddle.attach(rightPaddlePin);
  sepPipeGateServo.attach(sepPipeGate);
  screw.attach(screwServoPin);

  // Sets the pins for the limit switchs
  pinMode(limitSwitchPinOne, INPUT_PULLUP);
  pinMode(limitSwitchPinTwo, INPUT_PULLUP);
  pinMode(limitSwitchPinThree, INPUT_PULLUP);
  pinMode(limitSwitchPinFour, INPUT_PULLUP);


  pinMode(topLiftLimitSwitch, INPUT_PULLUP);
  pinMode(bottomLiftLimitSwitch, INPUT_PULLUP);


  //bottomAirLock.write(90);
  closePaddles();
}

/*
____    ____  ______    __   _______      __        ______     ______   .______   
\   \  /   / /  __  \  |  | |       \    |  |      /  __  \   /  __  \  |   _  \  
 \   \/   / |  |  |  | |  | |  .--.  |   |  |     |  |  |  | |  |  |  | |  |_)  | 
  \      /  |  |  |  | |  | |  |  |  |   |  |     |  |  |  | |  |  |  | |   ___/  
   \    /   |  `--'  | |  | |  '--'  |   |  `----.|  `--'  | |  `--'  | |  |      
    \__/     \______/  |__| |_______/    |_______| \______/   \______/  | _|      
                                                                                  
*/
// Runs in a loop
void loop() {
  //Read From Serial
  if (Serial.available()) {
    char receivedChar = Serial.read();
    if (receivedChar == '\n') {  // Correct the newline character
      parseData(receivedData);
      receivedData = "";  // Clear the string after parsing
    } else {
      receivedData += receivedChar;  // Accumulate serial data into the string
    }
  }

  Serial.println("Pin Status 1: ");
  Serial.print(digitalRead(limitSwitchPinOne);
  Serial.println("Pin Status 2: ");
  Serial.print(digitalRead(limitSwitchPinTwo);
  Serial.println("Pin Status 3: ");
  Serial.print(digitalRead(limitSwitchPinThree);
  Serial.println("Pin Status 4: ");
  Serial.print(digitalRead(limitSwitchPinFour);
}

//Resets the bot when in neutral mode
void resetBot() {

  //Reset Teleop control variables
  RightTrigger = 0;
  LeftTrigger = 0;
  LeftStick = 0;
  drive = 0;
  leftTurn = 0;
  rightTurn = 0;
  currentPin = 1;

  //Stop motors
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500);
  screw.writeMicroseconds(1500);

  closeTopAirLock();
  closeBottomAirLock();
  closeClamp();
  delay(1000);
  closePaddles();
  //openPaddles();
  

  colorSort.write(140);
  while (digitalRead(limitSwitchPinTwo) != 0) {}
  colorSort.write(90);

  colorSort.write(40);
  while (digitalRead(limitSwitchPinFour) != 0) {}
  colorSort.write(90);
}
//Find direction and moves towards red pos until Limit Switch is Low then open and close air locks
void toRed() {
  
  closeTopAirLock();

  if (indexfromkey("redPin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("redPin");
  } else if (indexfromkey("redPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("redPin");
  }
  else if (currentPin == indexfromkey("redPin")){
    innitPosRed = true;
  }


  while (true) {
    //Serial.println(digitalRead(redPin));
    if (digitalRead(redPin) == LOW or innitPosRed == true) {
      innitPosYel = false;
      colorSort.write(90);
      closeTopAirLock();
      delay(1000);
      openBottomAirLock();
      delay(1000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      break;
    }
  }
}
//Find direction and moves towards green pos until Limit Switch is Low then open and close air locks

void toGreen() {
  closeTopAirLock();

  // //Serial.println("inGreen");
  if (indexfromkey("greenPin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("greenPin");
  } else if (indexfromkey("greenPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("greenPin");
  }
  else if (currentPin == indexfromkey("greenPin")){
    //innitPos = true;
  }
  while (true) {
    if (digitalRead(greenPin) == LOW) {
      innitPosRed = false;
      innitPosYel = false;
      colorSort.write(90);
      closeTopAirLock();
      delay(1000);
      openBottomAirLock();
      delay(1000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      // closeTopAirLock();
      break;
    }
  }
  
}
//Find direction and moves towards blue pos until Limit Switch is Low then open and close air locks

void toBlue() {
  closeTopAirLock();
  if (indexfromkey("bluePin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("bluePin");
  } else if (indexfromkey("bluePin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("bluePin");
  }
  else if (currentPin == indexfromkey("bluePin")){
    //innitPos = true;
  }
  while (true) {
    if (digitalRead(bluePin) == LOW) {
      innitPosRed = false;
      innitPosYel = false;
      colorSort.write(90);
      closeTopAirLock();
      delay(1000);
      openBottomAirLock();
      delay(1000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      //delay(1000);
      // closeTopAirLock();
      break;
    }
  }
}
////Find direction and moves towards yellow pos until Limit Switch is Low then open and close air locks

void toYellow() {
  closeTopAirLock();
  if (indexfromkey("yellowPin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("yellowPin");
  } else if (indexfromkey("yellowPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("yellowPin");
  }
  else if (currentPin == indexfromkey("yellowPin")){
    innitPosYel = true;
  }
  while (true) {
    
    if ((digitalRead(yellowPin) == LOW) or innitPosYel == true) {     
      innitPosRed = false;
      colorSort.write(90);
      closeTopAirLock();
      delay(1000);
      openBottomAirLock();
      delay(1000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      // closeTopAirLock();
      break;
    }
  }
}

// opens Clamp
void openClamp() {

  if(!clampOpen){
    clamp.write(180);

    clampOpen = true;
  }

}
//close Clamp

void closeClamp() {
  if(clampOpen){
    clamp.write(0);

    clampOpen = false;
  }
}

void openPaddles() {
  leftPaddle.write(85);
  rightPaddle.write(125);
}
 
void closePaddles() {
  leftPaddle.write(0);
  rightPaddle.write(30);
}

//Parses the instuction recieved from the Pi

// moves robot Forward
void goForward() {
  leftservo.writeMicroseconds(1700);
  rightservo.writeMicroseconds(1700);
  delay(2000);
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500);
}
//turn robot around
void turnAround() {
  leftservo.writeMicroseconds(1200);
  rightservo.writeMicroseconds(1800);
  delay(2000);
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500);
}
//moves screw to top
void screwUp() {
    screw.writeMicroseconds(1000);

  reachedTop = true;
  delay(1000);
  reachedBottom = true;
}
// moving the screw motor down
void screwDown() {
  screw.writeMicroseconds(2000);

  reachedBottom = true;
  delay(1000);

  reachedTop = true;
}
