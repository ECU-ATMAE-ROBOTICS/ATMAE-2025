import logging
import time
import socket
import cv2
import struct
from datetime import datetime

SERVER_IP = '192.168.4.32'  # Change to the IP of the server
PORT = 9999

logger = logging.getLogger(__name__)

#Button ID to activate Neutral mode of bot
neutral_mode = 23

#Axis that are registered as valid input
validSticks = [5]
validTriggers = [9, 10]

#Keeps track of previous inputs sent by an ID to prevent serial clogging
prevInstructions = {5:0.0, 9:0.0, 10:0.0}

def teleop(controller, arduino):
    while True:
        # Detects and sends controller inputs
        instructions = controller.getControllerInput()

        #Checks if theres inputs from the controller
        if instructions != None:
        
            for instruction in instructions:
                instructionID = int(controller.getInputID(instruction))
                instructionValue = float(controller.getInputValue(instruction).strip())
                
                #Checks if the input is valid to send through serial
                if instructionID in validSticks or instructionID in validTriggers:
                    
                    if prevInstructions.get(instructionID) != instructionValue:
                        arduino.write(instruction.encode("utf-8"))

                    #Prevents repetitive values from taking up space in serial
                    prevInstructions[instructionID] = instructionValue
                
                #Neutral mode button pressed
                elif instructionID == neutral_mode:

                    #Alert arduino to go in neutral
                    arduino.write(instruction.encode("utf-8"))
                    return

                
        time.sleep(0.02)

def send_image(img):
    # encode image
    _, img_encoded = cv2.imencode('.jpg', img)
    img_bytes = img_encoded.tobytes()

    # Prefix message with 4-byte length
    msg = struct.pack('>I', len(img_bytes)) + img_bytes

    # Send the image
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((SERVER_IP, PORT))
        s.sendall(msg)

        # Receive response
        response = s.recv(1024)
        return response


def auto(controller, arduino):
    cap = cv2.VideoCapture(0)
    instruction = None
    neutral_pressed = False

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((SERVER_IP, PORT))
        logger.info(f"|{datetime.now().strftime('%H:%M:%S')}| Connected to server.")

        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break

            # Encode frame as JPEG
            _, buffer = cv2.imencode('.jpg', frame)
            frame_bytes = buffer.tobytes()

            # Send frame
            msg = struct.pack('>I', len(frame_bytes)) + frame_bytes
            s.sendall(msg)

            # Wait for response
            direction = s.recv(2)

            for instruction in controller.getControllerInput():
                if int(controller.getInputID(instruction)) == neutral_mode:
                    neutral_pressed = True
                    arduino.write(instruction.encode('utf-8'))
                    break

            if neutral_pressed:
                s.close()
                break
            else:
                print(direction)

                        

        cap.release()
        logger.info(f"|{datetime.now().strftime('%H:%M:%S')}| Stream has ended.")
   



    

