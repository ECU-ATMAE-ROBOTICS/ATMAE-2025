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

//---PINS---

const int rightDriveMotorPin = 9;
const int leftDriveMotorPin = 10;

const int screwServoPin = 44;
const int topLiftLimitSwitch = 52;
const int bottomLiftLimitSwitch = 1001;


const int forkLiftPin = 44;
const int leftPaddlePin = 11;
const int rightPaddlePin = 12;
const int clampPin = 13;

const int topAirLockPin = 2;
const int bottomAirLockPin = 3;

const int colorSortPin = 6;
const int limitSwitchPinOne = 37;
const int limitSwitchPinTwo = 31;
const int limitSwitchPinThree = 33;
const int limitSwitchPinFour = 35;

const int rightPaddle = 11;
const int leftPaddle = 12;

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

//Motor Servo
Servo leftservo;
Servo rightservo;

//clamp code
Servo clamp;
const int clampOpenPos = 1800;
const int clampClosePos = 1300;

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
const int sepPipeGate=999;
//Gate Servos
Servo rightPipeGateServo;
Servo midPipeGateServo;
Servo leftPipeGateServo;
Servo sepPipeGateServo;




//location  of  the drop points (placeholder values)
// const int sepPos = 150;
// const int Pos1 = 30;
// const int Pos2 = 60;
// const int Pos3 = 90;
// const int centerPos = 90;


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
int currentPin = 0;
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
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  // colorSort.write(sortSpeed);

  //Initialize the servos
  leftservo.attach(leftDriveMotorPin);
  rightservo.attach(rightDriveMotorPin);
  colorSort.attach(colorSortPin);
  clamp.attach(clampPin);
  topAirLock.attach(topAirLockPin);
  bottomAirLock.attach(bottomAirLockPin);

  // Sets the pins for the limit switchs
  pinMode(limitSwitchPinOne, INPUT_PULLUP);
  pinMode(limitSwitchPinTwo, INPUT_PULLUP);
  pinMode(limitSwitchPinThree, INPUT_PULLUP);
  pinMode(limitSwitchPinFour, INPUT_PULLUP);


  pinMode(topLiftLimitSwitch, INPUT_PULLUP);
  pinMode(bottomLiftLimitSwitch, INPUT_PULLUP);


  //attach the pipes
  rightPipeGateServo.attach(rightPipeGate);
  midPipeGateServo.attach(midPipeGate);
  leftPipeGateServo.attach(leftPipeGate);
  openTopAirLock();
  closeBottomAirLock();
  rightPipeGateServo.write(closeGatePosition);
  midPipeGateServo.write(closeGatePosition);
  leftPipeGateServo.write(closeGatePosition);
  screw.attach(screwServoPin);
  sepPipeGateServo.attach(sepPipeGate);
  // screwUp();

  //set color system slide all the way to first limitswitch
  colorSort.write(140);
  while (digitalRead(limitSwitchPinTwo) != 0) {}
  colorSort.write(90);
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


  bool reachedTop = true;
  if (digitalRead(topLiftLimitSwitch) == LOW and reachedTop) {
    screw.writeMicroseconds(1500);
    reachedTop = false;
  }

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
  currentPin = 0;

  //Stop motors
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500);
  screw.writeMicroseconds(1500);

  closeTopAirLock();
  closeBottomAirLock();

  colorSort.write(140);
  while (digitalRead(limitSwitchPinTwo) != 0) {}
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


  while (true) {
    if (digitalRead(redPin) == LOW) {
      colorSort.write(90);
      closeTopAirLock();
      delay(1000);
      openBottomAirLock();
      delay(1000);
      closeBottomAirLock();
      delay(1000);
      openTopAirLock();
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
  } else if (indexfromkey("greenPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("greenPin");
  }
  while (true) {
    if (digitalRead(greenPin) == LOW) {
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
  } else if (indexfromkey("bluePin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("bluePin");
  }
  while (true) {
    if (digitalRead(bluePin) == LOW) {
      colorSort.write(90);
      closeTopAirLock();
      delay(1000);
      openBottomAirLock();
      delay(1000);
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
////Find direction and moves towards yel    pos until Limit Switch is Low then open and close air locks

void toYellow() {
  closeTopAirLock();
  if (indexfromkey("yellowPin") < currentPin) {
    colorSort.write(140);
    currentPin = indexfromkey("yellowPin");
  } else if (indexfromkey("yellowPin") > currentPin) {
    colorSort.write(40);
    currentPin = indexfromkey("yellowPin");
  }
  while (true) {
    if (digitalRead(yellowPin) == LOW) {
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
  clamp.writeMicroseconds(clampOpenPos);
}
//close Clamp

void closeClamp() {
  clamp.writeMicroseconds(clampClosePos);
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
    }

    else if (button_id == Right_STICK_IDY) {
      if (axis_val > 0) {
        screwDown();
      } else if (axis_val < 0) {
        screwUp();
      }
    } else if (button_id == NEUTRAL_ID) {
      resetBot();
    } else {
      Serial.println("Err");
    }

  } else {
    Serial.println(data);
    // Error handling if data doesn't contain ':'

    //Check if data is a command
    //if data is sepColor then sets pos of colors
    if (data == "sepBlue") {
      colorPins[0] = "redPin";
      colorPins[1] = "yellowPin";
      colorPins[2] = "greenPin";
      colorPins[3] = "bluePin";
      bluePin = limitSwitchPinOne;
      redPin = limitSwitchPinTwo;
      yellowPin = limitSwitchPinThree;
      greenPin = limitSwitchPinFour;

      currentPin = indexfromkey("redPin");

      closeRed();
      closeGreen();
      closeBlue();
      closeYellow();

    } else if (data == "sepRed") {
      colorPins[0] = "bluePin";
      colorPins[1] = "yellowPin";
      colorPins[2] = "greenPin";
      colorPins[3] = "redPin";
      redPin = limitSwitchPinOne;
      bluePin = limitSwitchPinTwo;
      yellowPin = limitSwitchPinThree;
      greenPin = limitSwitchPinFour;

      currentPin = indexfromkey("bluePin");

      closeRed();
      closeGreen();
      closeBlue();
      closeYellow();

    } else if (data == "sepGreen") {
      colorPins[0] = "bluePin";
      colorPins[1] = "yellowPin";
      colorPins[2] = "redPin";
      colorPins[3] = "greenPin";

      currentPin = indexfromkey("bluePin");

      greenPin = limitSwitchPinOne;
      bluePin = limitSwitchPinTwo;
      yellowPin = limitSwitchPinThree;
      redPin = limitSwitchPinFour;

      closeRed();
      closeGreen();
      closeBlue();
      closeYellow();

    } else if (data == "sepYellow") {
      colorPins[0] = "bluePin";
      colorPins[1] = "redPin";
      colorPins[2] = "greenPin";
      colorPins[3] = "yellowPin";

      currentPin = indexfromkey("bluePin");

      yellowPin = limitSwitchPinOne;
      bluePin = limitSwitchPinTwo;
      redPin = limitSwitchPinThree;
      greenPin = limitSwitchPinFour;

      closeRed();
      closeGreen();
      closeBlue();
      closeYellow();

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
    } else if (data == "stopClamp") {
      clamp.writeMicroseconds(1500);
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
    } else if (data == "stopSort") {
      Serial.println("stop sort");

      stopSort();

    } else if (data == "goForward") {
      goForward();
    } else if (data == "turnAround") {
      turnAround();
    } else if (data == "screwUp") {
      Serial.println("DBWIE");
      screwUp();
    } else if (data == "screwDown") {
      screwDown();
    } else if (data == "shake") {
      shake();
    } else if (data == "screwStop") {
      screw.writeMicroseconds(1500);
    }
  }
}
//open all the pipes
void openBottom(int pin) {
  Serial.println(pin);
  if (pin == limitSwitchPinTwo) {
    rightPipeGateServo.write(90);
  }
  if (pin == limitSwitchPinThree) {
    midPipeGateServo.write(160);  //Hits right drive motor if at 180 :|
  }
  if (pin == limitSwitchPinFour) {
    leftPipeGateServo.write(90);
  }
  if(pin ==limitSwitchPinOne)
  {
    sepPipeGateServo.write(0);
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
    rightPipeGateServo.write(180);
  }
  if (pin == limitSwitchPinThree) {
    midPipeGateServo.write(90);
  }
  if (pin == limitSwitchPinFour) {
    leftPipeGateServo.write(0);
  }
  if(pin ==limitSwitchPinOne)
  {
    sepPipeGateServo.write(180);
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
void stopSort() {
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
  //while (digitalRead(topLiftLimitSwitch) != 0){}

  // screw.writeMicroseconds(1500);
}
// moving the screw motor down
void screwDown() {
  screw.writeMicroseconds(2000);

  //while (digitalRead(topLiftLimitSwitch) != 0){}

  //screw.writeMicroseconds(1500);
}
//shake robot
void shake() {
  leftservo.writeMicroseconds(1300);
  rightservo.writeMicroseconds(1300);
  delay(500);
  leftservo.writeMicroseconds(1700);
  rightservo.writeMicroseconds(1700);
  delay(500);
  leftservo.writeMicroseconds(1500);
  rightservo.writeMicroseconds(1500);
}