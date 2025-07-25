from ControllerCommon.XboxController import XboxController
import robotmodes

import serial
import time
import logging
import pygame
import os
from datetime import datetime

logger = logging.getLogger(__name__)
log_path = "/home/ubuntu/Desktop/ATMAE-2025/RaspberryPi/logs" 

try:
        if __name__ == "__main__":
            
            #Removing old log files
            for file in os.listdir(log_path):
                file_path = os.path.join(log_path, file)
                if os.path.isfile(file_path):
                    os.remove(file_path)

            log_path = log_path + r'/main.log'
            logging.basicConfig(filename=log_path, level=logging.INFO)
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Started\n[bleh]")

            arduino_connected = False
            arduino = None

            controller_connected = False
            controller = None


            # Connect to the Xbox Controller
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Attempting to connect to controller")
            while not controller_connected:
                try:
                    pygame.init()
                    controller = XboxController(deadZone=0.2)
                    controller_connected = True
                
                except pygame.error:
                    logger.error(f"|{datetime.now().strftime('%H:%M:%S')}|Couldn't connect to controller, trying again in 1 sec")
                    pygame.joystick.quit()
                    time.sleep(1)
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Connected to controller!")

            # Connect to Arduino
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Attempting to connect to arduino")
            while not arduino_connected:
                try:
                    arduino = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
                    arduino.reset_input_buffer()
                    arduino.reset_output_buffer()
                    arduino_connected = True
                except serial.serialutil.SerialException:
                    logging.error(f"|{datetime.now().strftime('%H:%M:%S')}|Couldn't connect to Arduino, trying again in 1 sec")
                    time.sleep(1)
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Connected to arduino!")


            #Testing serial communication
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Testing serial communication")

            while arduino.in_waiting == 0:
                arduino.write("Testing\n".encode("utf-8"))
                time.sleep(1)

            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Serial testing successfull! Message from arduino: {arduino.readline().rstrip()}")


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
                                logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|In auto mode")
                                robotmodes.auto(controller, arduino)
                                logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|In neutral mode")

                            #Menu Button
                            case 22:
                                logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|In teleop mode")
                                robotmodes.teleop(controller, arduino)
                                logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|In neutral mode")


except KeyboardInterrupt:
    if arduino != None:    
        arduino.close()
    pygame.quit()

    logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Program cancelled by user")
    exit()



