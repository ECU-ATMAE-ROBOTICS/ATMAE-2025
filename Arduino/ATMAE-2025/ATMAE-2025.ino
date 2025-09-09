//ssh ubuntu@192.168.0.157
// 192.168.0.228
//port 9000

#define LED_PIN 11

#include <Servo.h>

// ID's of controller buttons
const int TELEOP_ID = 22;
const int AUTO_ID = 21;
const int NEUTRAL_ID = 23;
const int LEFT_TRIGGER_ID = 9;
const int RIGHT_TRIGGER_ID = 10;
const int LEFT_STICK_ID = 5;

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


Servo colorSort;
//placeholder values
int  redPos=30;
int  yellowPos=60;
int  greenPos=120;
int  bluePos=150;
const int centerPos=90;





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
  leftservo.attach(2);
  rightservo.attach(3);
  colorSort.attach(4);
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
    if(abs(drive) > 0){
      RMotor = 1500 + 500 * drive * (1 - rightTurn);
      LMotor = 1500 - 500 * drive * (1 - leftTurn);//Motor is inverted
    }
    //Turn in place
    else if (button_id == 5){
      RMotor = 1500 + 500 * LeftStick;
      LMotor = 1500 + 500 * LeftStick;
    }

    //Set motor speed
    leftservo.writeMicroseconds(LMotor);
    rightservo.writeMicroseconds(RMotor);

  if (Auto){
    1;
  }
}

//Resets the bot when in neutral mode
void resetBot(){

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

//Parses the instuction recieved from the Pi
void parseData(String data) {
  int splitIndex = data.indexOf(':');  // Find where the ';' is
  if (splitIndex != -1) {              // Ensure ';' exists in the data
    String buttonStr = data.substring(0, splitIndex);
    String axisStr = data.substring(splitIndex + 1);

    // Convert to int and double
    button_id = buttonStr.toInt(); //ID of the input
    axis_val = axisStr.toDouble(); //Value of the input

    //Forwards (RightTrigger)
    if (button_id == RIGHT_TRIGGER_ID) {
      RightTrigger = (1 + axis_val) / 2;

      if (RightTrigger < .2){
        RightTrigger = 0;
      }
    } 
    //Reverse (Left Trigger)
    else if (button_id == LEFT_TRIGGER_ID) {
      LeftTrigger = (1 + axis_val) / 2;

      if (LeftTrigger < .2){
        LeftTrigger = 0;
      }
    }

    //Turning (Left-Stick X axis)
    else if (button_id == LEFT_STICK_ID) {
      LeftStick = axis_val;

      //Turn right
      if (axis_val > 0){
        rightTurn = axis_val;
        leftTurn = 0;
      }
      //Turn left
      else if (axis_val < 0){
        rightTurn = 0;
        leftTurn = -axis_val;
      }
      //Full Stop
      else{
        rightTurn = 0;
        leftTurn = 0;
      }
    }

    else if (button_id == NEUTRAL_ID){
      resetBot();
      Auto = false;
    }

    else if (button_id == AUTO_ID){
      resetBot();
      Auto = true;
      
    }

  } else {
    // Error handling if data doesn't contain ':'
    if(data=="sepBlue")
    {
        bluePos=150;
        redPos=30;
        yellowPos=60;
        greenPos=120;
    }
    else  if(data=="sepRed")
    {
        redPos=150;
        bluePos=30;
        yellowPos=60;
        greenPos=120;
    }
    else if(data=="sepGreen")
    {
        greenPos=150;
        bluePos=30;
        yellowPos=60;
        redPos=120;
    }
    else if(data=="sepGreen")
    {
        yellowPos=150;
        bluePos=30;
        greenPos=60;
        redPos=120;
    }

    else if(data=="toRed")
    {
        toRed();
    }
    else if(data=="toBlue")
    {
        toBlue();
    }
    else if(data=="toGreen")
    {
        toGreen();
    }
    else if(data=="toYellow")
    {
        toYellow();
    }
    else if(data="toCenter")
    {
        toCenter();
    }
    else
    {
            Serial.println("Err");

    }
  
  }
}


void toRed()
{
  colorSort.write(redPos);
}

void toGreen()
{
  colorSort.write(greenPos);
}

void toBlue()
{
  colorSort.write(bluePos);
}

void toYellow()
{
  colorSort.write(yellowPos);
}
void toCenter()
{
  colorSort.write(centerPos);
}