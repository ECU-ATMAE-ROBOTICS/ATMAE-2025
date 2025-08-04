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

//Ratio to slow each motor for turning
double turnValue = 0;

//PWM values passed to the motors
double LMotor = 1500;
double RMotor = 1500;

bool Teleop = false;
bool Auto = false;

Servo leftservo;
Servo rightservo;

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

  

  if (Teleop){
    LMotor = 1500 - 500 * (RightTrigger - LeftTrigger) + 500 * turnValue;
    RMotor = 1500 + 500 * (RightTrigger - LeftTrigger) + 500 * turnValue;

    leftservo.writeMicroseconds(LMotor);
    rightservo.writeMicroseconds(RMotor);
  }
  else if (Auto){
    1;
  }


}

//Resets the bot when in neutral mode
void resetBot(){

  //Reset Teleop control variables
  RightTrigger = 0;
  LeftTrigger = 0;
  turnValue = 0;
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
      turnValue = axis_val;
    }

    else if (button_id == NEUTRAL_ID && axis_val == 1.0){
      resetBot();
      Teleop = false;
      Auto = false;
    }

    else if (button_id == AUTO_ID && axis_val == 1.0){
      Auto = true;
      Teleop = false; // just in case
    }

    else if (button_id == TELEOP_ID && axis_val == 1.0){
      Teleop = true;
      Auto = false; // just in case
    }


  } else {
    // Error handling if data doesn't contain ':'
    Serial.println("Err");
  }
}
