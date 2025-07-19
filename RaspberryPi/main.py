from ControllerCommon.XboxController import XboxController
import robotmodes

import serial
import time
import logging
import pygame

logger = logging.getLogger(__name__)

if __name__ == "__main__":
    logging.basicConfig(filename='/home/ecu/Desktop/ATMAE-2025/RaspberryPi/logs/main.log', level=logging.INFO)
    logger.info("Started\n[bleh]")


    arduino_connected = False
    arduino = None

    controller_connected = False
    controller = None


    # Connect to the Xbox Controller
    logger.info("Attempting to connect to controller")
    while not controller_connected:
        try:
            pygame.init()
            controller = XboxController(deadZone=0.35)
            controller_connected = True
        
        except pygame.error:
            logger.error("Couldn't connect to controller, trying again in 1 sec")
            pygame.joystick.quit()
            time.sleep(1)
    logger.info("Connected to controller!")
    
    # Connect to Arduino
    logger.info("Attempting to connect to arduino")
    while not arduino_connected:
        try:
            arduino = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
            arduino.reset_input_buffer()
            arduino.reset_output_buffer()
            arduino_connected = True
        except serial.serialutil.SerialException:
            logging.error("Couldn't connect to Arduino, trying again in 1 sec")
            time.sleep(1)
    logger.info("Connected to arduino!")


    #Testing serial communication
    logger.info("Testing serial communication")
    
    while arduino.in_waiting == 0:
        arduino.write("Testing\n".encode("utf-8"))
        time.sleep(1)
    
    logger.info(f"Serial testing successfull! Message from arduino: {arduino.readline().rstrip()}")


    
    while True:
        #Select robot mode
        instructions = controller.getControllerInput()
        if instructions != None:    
            for instruction in instructions:
                print(instruction)
                inputID = int(controller.getInputID(instruction))
 
                match inputID:
                    #View Button
                    case 21:
                        logger.info("In auto mode")
                        robotmodes.auto(controller, arduino)
                        logger.info("In neutral mode")

                    #Menu Button
                    case 22:
                        logger.info("In teleop mode")
                        robotmodes.teleop(controller, arduino)
                        logger.info("In neutral mode")


