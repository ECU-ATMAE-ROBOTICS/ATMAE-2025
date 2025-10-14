//ssh ubuntu@192.168.0.157
// 192.168.0.228
//port 9000

#define LED_PIN 11

#include <Servo.h>
#include <Stepper.h>
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

Servo leftservo;
Servo rightservo;

//clamp code
Servo clamp;
const int clampPin = 10;
const int clampOpenPos = 90;
const int clampClosePos = 10;

Servo colorSort;


//placeholder values for color pos changed later
int sortSpeed = 120;
int redPin = 30;
int yellowPin = 60;
int greenPin = 120;
int bluePin = 150;
const int limtSwitchPinOne = 22;
const int limtSwitchPinTwo = 23;
const int limtSwitchPinThree = 24;
const int limtSwitchPinFour = 25;
//bottom servos for colorsort 
Servo botTwo;
Servo botThree;
Servo botFour;
const int botOpen=0;
const int botClose=180;
const int centerPos = 90;
//location  of  the drop points (placeholder values)
const int sepPos = 150;
const int Pos1 = 30;
const int Pos2 = 60;
const int Pos3 = 90;
//stepper mortor
const int StepPin = 28;
const int DirPin = 26;

AccelStepper stepper(AccelStepper::DRIVER, StepPin, DirPin);
const int maxStepperPos = 100;
const int minStepperPos = 10;
const int upAmount = 100;
const int downAmount = 10;
//Servos for open close
Servo topSorter;
Servo bottomSorter;
const int topSorterPin = 12;
const int bottomSorterPin = 13;
double RightStick = 0;



/*
Parses instructions from the PI
to determine robot movement
*/
void parseData(String data);


void setup() {

  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);




  //Initialize the servos
  leftservo.attach(6);
  rightservo.attach(7);
  colorSort.attach(4);
  stepper.setMaxSpeed(200.0);
  stepper.setAcceleration(100.0);
  clamp.attach(clampPin);
  topSorter.attach(topSorterPin);
  bottomSorter.attach(bottomSorterPin);
  pinMode(limtSwitchPinOne, INPUT_PULLUP);
  pinMode(limtSwitchPinTwo, INPUT_PULLUP);
  pinMode(limtSwitchPinThree, INPUT_PULLUP);
  pinMode(limtSwitchPinFour, INPUT_PULLUP);
  botTwo.attach(8);
  botThree.attach(9);
  botFour.attach(10);
}

/*
____    ____  ______    __   _______      __        ______     ______   .______   
\   \  /   / /  __  \  |  | |       \    |  |      /  __  \   /  __  \  |   _  \  
 \   \/   / |  |  |  | |  | |  .--.  |   |  |     |  |  |  | |  |  |  | |  |_)  | 
  \      /  |  |  |  | |  | |  |  |  |   |  |     |  |  |  | |  |  |  | |   ___/  
   \    /   |  `--'  | |  | |  '--'  |   |  `----.|  `--'  | |  `--'  | |  |      
    \__/     \______/  |__| |_______/    |_______| \______/   \______/  | _|      
                                                                                  
*/

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

  if (Auto) {
    1;
  }
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

void toRed() {
  colorSort.write(sortSpeed);
  while (digitalRead(redPin) == HIGH) {
  }
  if (digitalRead(redPin) == LOW) {
    colorSort.write(90);
  }
}

void toGreen() {
  colorSort.write(sortSpeed);
  while (digitalRead(greenPin) == HIGH) {
  }
  if (digitalRead(greenPin) == LOW) {
    colorSort.write(90);
    //open hatch later
  }
}

void toBlue() {
  colorSort.write(sortSpeed);
  while (digitalRead(bluePin) == HIGH) {
  }
  if (digitalRead(bluePin) == LOW) {
    colorSort.write(90);
    //open hatch later
  }
}

void toYellow() {
  colorSort.write(sortSpeed);
  while (digitalRead(yellowPin) == HIGH) {
  }
  if (digitalRead(yellowPin) == LOW) {
    colorSort.write(90);
    //open hatch later
  }
}
void toCenter() {
  colorSort.write(centerPos);
}
void openClamp() {
  clamp.write(clampOpenPos);
}
void closeClamp() {
  clamp.write(clampClosePos);
}
void stepperMoveDown() {
  // stepper.moveTo(minStepperPos);
  stepper.moveTo(downAmount);
  stepper.run();
}
void stepperMoveUp() {
  stepper.moveTo(upAmount);

  // stepper.moveTo(maxStepperPos);
  stepper.run();
}



//Parses the instuction recieved from the Pi
void parseData(String data) {
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
    //Stepper Motor Movemnet
    else if (button_id == Right_STICK_ID) {
      RightStick = axis_val;

      //Go Down
      if (axis_val > 0) {
        stepperMoveDown();
      }
      //Go up
      else if (axis_val < 0) {
        stepperMoveUp();
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




    else {
      // Error handling if data doesn't contain ':'
      if (data == "sepBlue") {
        bluePin = limtSwitchPinOne;
        redPin = limtSwitchPinTwo;
        yellowPin = limtSwitchPinThree;
        greenPin = limtSwitchPinFour;
      } else if (data == "sepRed") {
        redPin = limtSwitchPinOne;
        bluePin = limtSwitchPinTwo;
        yellowPin = limtSwitchPinThree;
        greenPin = limtSwitchPinFour;


      } else if (data == "sepGreen") {
        redPin = limtSwitchPinFour;
        bluePin = limtSwitchPinTwo;
        yellowPin = limtSwitchPinThree;
        greenPin = limtSwitchPinOne;
      } else if (data == "sepYellow") {
        redPin = limtSwitchPinFour;
        bluePin = limtSwitchPinTwo;
        yellowPin = limtSwitchPinOne;
        greenPin = limtSwitchPinThree;
      }

      else if (data == "toRed") {
        toRed();
      } else if (data == "toBlue") {
        toBlue();
      } else if (data == "toGreen") {
        toGreen();
      } else if (data == "toYellow") {
        toYellow();
      } else if (data = "toCenter") {
        toCenter();
      } else if (data = "closeClamp") {
        closeClamp();
      } else if (data = "openClamp") {
        openClamp();
      } else if (data = "steppUP") {
        stepperMoveUp();
      } else if (data = "steppDown") {
        stepperMoveDown();
      } else if(data = "openRed") {
        openRed();
      } else if(data = "openGreen") {
        openGreen();
      } else if(data = "openBlue") {
        openBlue();
      } else if(data = "openYelllow") {
        openYellow();
      } else if(data = "closeRed") {
        closeRed();
      } else if(data = "closeGreen") {
        closeGreen();
      } else if(data = "closeBlue") {
        closeBlue();
      } else if(data = "closeYellow") {
        closeYellow();
      } else {
        Serial.println("Err");
      }
    }
  }
}
void openBottom(int pin)
{
  if(pin==limtSwitchPinTwo)
  {
    botTwo.write(botOpen);
  }
  if(pin==limtSwitchPinThree)
  {
    botThree.write(botOpen);
  }
    if(pin==limtSwitchPinFour)
  {
    botFour.write(botOpen);
  }
  
}
void openRed()
{
  openBottom(redPin);
}
void openGreen()
{
  openBottom(greenPin);
}
void openBlue()
{
  openBottom(bluePin);
}
void openYellow()
{
  openBottom(yellowPin);
}
void closeBottom(int pin)
{
  if(pin==limtSwitchPinTwo)
  {
    botTwo.write(botClose);
  }
  if(pin==limtSwitchPinThree)
  {
    botThree.write(botClose);
  }
    if(pin==limtSwitchPinFour)
  {
    botFour.write(botClose);
  }
  
}
void closeRed()
{
  closeBottom(redPin);
}
void closeGreen()
{
  closeBottom(greenPin);
}
void closeBlue()
{
  closeBottom(bluePin);
}
void closeYellow()
{
  closeBottom(yellowPin);
}