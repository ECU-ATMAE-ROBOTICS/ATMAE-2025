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

#define LED_PIN 11

#include <Servo.h>
#include <AccelStepper.h>

// ID's of controller buttons
const int TELEOP_ID = 22;
const int AUTO_ID = 21;
const int NEUTRAL_ID = 23;
const int LEFT_TRIGGER_ID = 9;
const int RIGHT_TRIGGER_ID = 10;
const int LEFT_STICK_ID = 5;
const int Right_STICK_ID = 8;
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

//Motor Servo
Servo leftservo;
Servo rightservo;

//clamp code
Servo clamp;
const int clampPin = 10;
const int clampOpenPos = 90;
const int clampClosePos = 10;




// Defines  the Smart Servo Sorting that move the color sorter 
Servo colorSort;


//placeholder values for color pos changed later
int sortSpeed = 70;
int redPin = 30;
int yellowPin = 60;
int greenPin = 120;
int bluePin = 150;
const int limitSwitchPinOne = 37;
const int limitSwitchPinTwo = 35;
const int limitSwitchPinThree = 33;
const int limitSwitchPinFour = 31;

//limit Switchs and Servo for the Lifting System 
Servo screw;
const int limitSwitchPinFive = 999;
const int limitSwitchPinSix = 1001;



//Pins for each servo gate
const int leftPipeGate = 5;
const int midPipeGate = 7;
const int rightPipeGate = 4;

//Gate Servos
Servo rightPipeGateServo;
Servo midPipeGateServo;
Servo leftPipeGateServo;




//location  of  the drop points (placeholder values)
// const int sepPos = 150;
// const int Pos1 = 30;
// const int Pos2 = 60;
// const int Pos3 = 90;
// const int centerPos = 90;


//Airlock Servos
Servo topAirLock;
Servo bottomAirLock;
const int topAirLockPin = 2;
const int bottomAirLockPin = 3;

//Positions for servos to open and close
const int openGatePosition = 180;
const int closeGatePosition = 90;

const int openTopAirLockPosition = 90;
const int openLowerAirLockPosition = 180;

const int closeTopAirLockPosition = 180;
const int closeLowerAirLockPosition = 90;

double RightStick = 0;

// Variable to hold the color being sorted
int currentPin = 2;
String colorPins[4];

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
Close = 90
Open = 0

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
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  // colorSort.write(sortSpeed);

  //Initialize the servos
  leftservo.attach(10);
  rightservo.attach(9);
  colorSort.attach(6);
  clamp.attach(clampPin);
  topAirLock.attach(topAirLockPin);
  bottomAirLock.attach(bottomAirLockPin);
  // Sets the pins for the limit switchs 
  pinMode(limitSwitchPinOne, INPUT_PULLUP);
  pinMode(limitSwitchPinTwo, INPUT_PULLUP);
  pinMode(limitSwitchPinThree, INPUT_PULLUP);
  pinMode(limitSwitchPinFour, INPUT_PULLUP);
  pinMode(limitSwitchPinFive, INPUT_PULLUP);
  pinMode(limitSwitchPinSix, INPUT_PULLUP);


  //attach the pipes
  rightPipeGateServo.attach(rightPipeGate);
  midPipeGateServo.attach(midPipeGate);
  leftPipeGateServo.attach(leftPipeGate);
  openTopAirLock();
  closeBottomAirLock();
  rightPipeGateServo.write(closeGatePosition);
  midPipeGateServo.write(closeGatePosition);
  leftPipeGateServo.write(closeGatePosition);
  screw.attach(44);
  screwUp();

  //bottomAirLock.write(90);
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

  drive = RightTrigger - LeftTrigger;

  //1500 is the stop value for both motors
  RMotor = 1500;
  LMotor = 1500;

  //Drive and Turn
  if (abs(drive) > 0) {
    RMotor = 1500 - 500 * drive * (1 - rightTurn);
    LMotor = 1500 - 500 * drive * (1 - leftTurn);  //Motor is inverted
  }
  //Turn in place
  else if (button_id == 5) {
    RMotor = 1500 - 500 * LeftStick;
    LMotor = 1500 + 500 * LeftStick;
  }

  //Set motor speed
  leftservo.writeMicroseconds(LMotor);
  rightservo.writeMicroseconds(RMotor);
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

  //Stop motors
  LMotor = 1500;
  RMotor = 1500;
  leftservo.writeMicroseconds(LMotor);
  rightservo.writeMicroseconds(RMotor);
}
//Find direction and moves towards red pos until Limit Switch is Low then open and close air locks
void toRed() { 
    closeTopAirLock();
  
  if (indexfromkey("redPin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("redPin");
  }
  else if(indexfromkey("redPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("redPin");
  }

  
  while(true) {
    if (digitalRead(redPin) == LOW) {
      colorSort.write(90);
      closeTopAirLock();
      delay(2000);
      openBottomAirLock();
      delay(2000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      delay(1000);
      //closeTopAirLock();
      break;
    } 
  }
  // colorSort.write(120);
  // while (digitalRead(redPin) == HIGH) {
  // }
  // if (digitalRead(redPin) == LOW) {
  //   colorSort.write(90);
  //   openBottomAirLock();
  //   delay(1000);
  //   closeBottomAirLock();
  // }
}
//Find direction and moves towards green pos until Limit Switch is Low then open and close air locks

void toGreen() {
    closeTopAirLock();

  // Serial.println("inGreen");
  if (indexfromkey("greenPin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("greenPin");
  }
  else if(indexfromkey("greenPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("greenPin");
  }
  while(true)
  {
    if (digitalRead(greenPin) == LOW) 
    {
      colorSort.write(90);
      closeTopAirLock();
      delay(2000);
      openBottomAirLock();
      delay(2000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      delay(1000);
      // closeTopAirLock();

      break;
     }

  }
  // colorSort.write(180);
  // while (digitalRead(greenPin) == HIGH) {
  // }
  // if (digitalRead(greenPin) == LOW) {
  //   colorSort.write(90);
  //   openBottomAirLock();
  //   delay(1000);
  //   closeBottomAirLock();
  // }
}
//Find direction and moves towards blue pos until Limit Switch is Low then open and close air locks

void toBlue() {
    closeTopAirLock();
   if (indexfromkey("bluePin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("bluePin");
  }
  else if(indexfromkey("bluePin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("bluePin");
  }
  while(true)
  {
    if (digitalRead(bluePin) == LOW) 
    {
      colorSort.write(90);
      closeTopAirLock();
      delay(2000);
      openBottomAirLock();
      delay(2000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      delay(1000);
      // closeTopAirLock();

      break;
     }
    

  }
  // colorSort.write(sortSpeed);
  // while (digitalRead(bluePin) == HIGH) {
  // }
  // if (digitalRead(bluePin) == LOW) {
  //   colorSort.write(90);
  //   openBottomAirLock();
  //   delay(1000);
  //   closeBottomAirLock();
  // }
}
////Find direction and moves towards yellow pos until Limit Switch is Low then open and close air locks

void toYellow() {
    closeTopAirLock();
   if (indexfromkey("yellowPin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("yellowPin");
  }
  else if(indexfromkey("yellowPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("yellowPin");
  }
  while(true)
  {
    if (digitalRead(yellowPin) == LOW) 
    {
      colorSort.write(90);
      closeTopAirLock();
      delay(2000);
      openBottomAirLock();
      delay(2000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
      delay(1000);
      // closeTopAirLock();

      break;
     }
     

  }
  // while (digitalRead(yellowPin) == HIGH||limtSwitchPinOne==HIGH) {
  //   if (limtSwitchPinOne == LOW) {
  //      colorSort.write(80);
  //   }
  // }
  // if (digitalRead(yellowPin) == LOW) {
  //   colorSort.write(90);
  //   openBottomAirLock();
  //   delay(1000);
  //   closeBottomAirLock();
  // }
}

// opens Clamp
void openClamp() {
  clamp.write(clampOpenPos);
}
//close Clamp

void closeClamp() {
  clamp.write(clampClosePos);
}




//Parses the instuction recieved from the Pi
void parseData(String data) {
  //Serial.println(data);
  int splitIndex = data.indexOf(':');  // Find where the ';' is
  if (splitIndex != -1) {              // Ensure ';' exists in the data
    String buttonStr = data.substring(0, splitIndex);
    String axisStr = data.substring(splitIndex + 1);

    // Convert to int and double
    button_id = buttonStr.toInt();  //ID of the input
    axis_val = axisStr.toDouble();  //Value of the input

    //Forwards (RightTrigger)
    if (button_id == RIGHT_TRIGGER_ID) {
      RightTrigger = (1 + axis_val) / 2;

      if (RightTrigger < .2) {
        RightTrigger = 0;
      }
    }
    //Reverse (Left Trigger)
    else if (button_id == LEFT_TRIGGER_ID) {
      LeftTrigger = (1 + axis_val) / 2;

      if (LeftTrigger < .2) {
        LeftTrigger = 0;
      }
    }

    //Turning (Left-Stick X axis)
    else if (button_id == LEFT_STICK_ID) {
      LeftStick = axis_val;

      //Turn right
      if (axis_val > 0) {
        rightTurn = axis_val;
        leftTurn = 0;
      }
      //Turn left
      else if (axis_val < 0) {
        rightTurn = 0;
        leftTurn = -axis_val;
      }
      //Full Stop
      else {
        rightTurn = 0;
        leftTurn = 0;
      }
    }
    //Clamp Movement
    else if (button_id == Right_STICK_IDX) {
      if (axis_val > 0) {
        openClamp();
      } else if (axis_val < 0) {
        closeClamp();
      }
    } else {
      Serial.println("Err");
    }

  } else {
    Serial.println(data);
    // Error handling if data doesn't contain ':'
    
    //Check if data is a command
    //if data is sepColor then sets pos of colors 
    if (data == "sepBlue") {
      colorPins[0] = "bluePin";
      colorPins[1] = "redPin";
      colorPins[2] = "yellowPin";
      colorPins[3] = "greenPin";
      bluePin = limitSwitchPinOne;
      redPin = limitSwitchPinTwo;
      yellowPin = limitSwitchPinThree;
      greenPin = limitSwitchPinFour;

    } else if (data == "sepRed") {
      colorPins[0] = "redPin";
      colorPins[1] = "bluePin";
      colorPins[2] = "yellowPin";
      colorPins[3] = "greenPin";
      redPin = limitSwitchPinOne;
      bluePin = limitSwitchPinTwo;
      yellowPin = limitSwitchPinThree;
      greenPin = limitSwitchPinFour;

    } else if (data == "sepGreen") {
      colorPins[0] = "greenPin";
      colorPins[1] = "bluePin";
      colorPins[2] = "redPin";
      colorPins[3] = "yellowPin";
      greenPin = limitSwitchPinOne;
      bluePin = limitSwitchPinTwo;
      yellowPin = limitSwitchPinThree;
      redPin = limitSwitchPinFour;

    } else if (data == "sepYellow") {
      colorPins[0] = "yellowPin";
      colorPins[1] = "bluePin";
      colorPins[2] = "redPin";
      colorPins[3] = "greenPin";
      yellowPin = limitSwitchPinOne;
      bluePin = limitSwitchPinTwo;
      redPin = limitSwitchPinThree;
      greenPin = limitSwitchPinFour;

    } else if (data == "toRed") {
      toRed();
    } else if (data == "toBlue") {
      toBlue();
    } else if (data == "toGreen") {
      toGreen();
    } else if (data == "toYellow") {
      toYellow();
    } else if (data == "closeClamp") {
      closeClamp();
    } else if (data == "openClamp") {
      openClamp();
    } else if (data == "openRed") {
      openRed();
    } else if (data == "openGreen") {
      openGreen();
    } else if (data == "openBlue") {
      openBlue();
    } else if (data == "openYellow") {
      openYellow();
    } else if (data == "closeRed") {
      closeRed();
    } else if (data == "closeGreen") {
      closeGreen();
    } else if (data == "closeBlue") {
      closeBlue();
    } else if (data == "closeYellow") {
      closeYellow();
    } else if (data == "openTopAirLock") {
      //Serial.println("Opening top airlock");
      openTopAirLock();
    } else if (data == "openBottomAirLock") {
      //Serial.println("Opening bottom airlock");
      openBottomAirLock();
    } else if (data == "closeTopAirLock") {
      //Serial.println("closing top airlock");
      closeTopAirLock();
    } else if (data == "closeBottomAirLock") {
      //Serial.println("closing bottom airlock");
      closeBottomAirLock();
    } else if(data=="stopSort"){
        Serial.println("stop sort");

        stopSort();

    }
    else if(data="goForward")
    {
      goForward();     
    }
    else if(data="turnAround")
    {
      turnAround();
    }
    else if(data=="screwUp")
    {
      screwUp();
    }
    else if(data=="screwDown")
    {
      screwDown();
    }
    else if(data=="shake")
    {
      shake();
    }
  }
}
//open all the pipes
void openBottom(int pin) {
  Serial.println(pin);
  if (pin == limitSwitchPinTwo) {
    rightPipeGateServo.write(openGatePosition);
  }
  if (pin == limitSwitchPinThree) {
    midPipeGateServo.write(openGatePosition);
  }
  if (pin == limitSwitchPinFour) {
    leftPipeGateServo.write(0);
  }
}
//opens red pipe
void openRed() {
  openBottom(redPin);
}
//opens green pipe
void openGreen() {
  openBottom(greenPin);
}
//opens blue pipe
void openBlue() {
  openBottom(bluePin);
}
//open yellow pipe
void openYellow() {
  openBottom(yellowPin);
}
//close bottom pipes
void closeBottom(int pin) {
  if (pin == limitSwitchPinTwo) {
    rightPipeGateServo.write(closeGatePosition);
  }
  if (pin == limitSwitchPinThree) {
    midPipeGateServo.write(closeGatePosition);
  }
  if (pin == limitSwitchPinFour) {
    leftPipeGateServo.write(closeGatePosition);
  }
}
void closeRed() {
  closeBottom(redPin);
}
void closeGreen() {
  closeBottom(greenPin);
}
void closeBlue() {
  closeBottom(bluePin);
}
void closeYellow() {
  closeBottom(yellowPin);
}
//opens top air lock
void openTopAirLock() {
  topAirLock.write(openTopAirLockPosition);
}
//close top air lock
void closeTopAirLock() {
  topAirLock.write(closeTopAirLockPosition);
}
//opens bottom air lock

void openBottomAirLock() {
  bottomAirLock.write(openLowerAirLockPosition);
}
//close bottom air lock
void closeBottomAirLock() {
  bottomAirLock.write(closeLowerAirLockPosition);
}
//stops the colorSort servo  
void stopSort()
{
  colorSort.write(90);
}
// gets index of elment in array
int indexfromkey(String key) {
  for (int i = 0; i < 4; i++) {
    if (colorPins[i] == key) {
      return i;
    }
  }
  return -1;
}
// moves robot Forward
void goForward()
{
  leftservo.writeMicroseconds(1200);
  rightservo.writeMicroseconds(1200);
  delay(2000);
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500);
}
//turn robot around 
void turnAround()
{
  leftservo.writeMicroseconds(1200);
  rightservo.writeMicroseconds(1800);
  delay(2000);
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500);
}
//moves screw to top
void screwUp()
{
    screw.writeMicroseconds(2000);

    if (digitalRead(limitSwitchPinFive) == LOW) {
          screw.writeMicroseconds(1500);

    } 
}
// moving the screw motor down
void screwDown()
{
    screw.writeMicroseconds(1000);

    if (digitalRead(limitSwitchPinSix) == LOW) {
          screw.writeMicroseconds(1500);

    } 
}
//shake robot 
void shake()
{
  leftservo.writeMicroseconds(1300);
  rightservo.writeMicroseconds(1300);
  delay(500);
  leftservo.writeMicroseconds(1700);
  rightservo.writeMicroseconds(17000);
  delay(500);
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500); 
}