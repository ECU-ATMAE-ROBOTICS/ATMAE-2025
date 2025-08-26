import logging
import time
import socket
import cv2
import struct
from datetime import datetime
import threading
import queue

SERVER_IP = '192.168.4.32'  # Change to the IP of the server
PORT = 9999

logger = logging.getLogger(__name__)

#Button ID to activate Neutral mode of bot
neutral_mode = 23

LSTICK_TURN = 5
LEFT_TRIGGER = 10
RIGHT_TRIGGER = 9

#Keeps track of previous inputs sent by an ID to prevent serial clogging
prevInstructions = {5:0.0, 9:0.0, 10:0.0}

stop_threads = threading.Event()
thread_pipe = queue.Queue()


def teleop(controller, arduino):

    arduino.write("22:1\n".encode("utf-8"))
    
    while True:
        # Detects and sends controller inputs
        instructions = controller.getControllerInput()

        #Checks if theres inputs from the controller
        if instructions != None:
        
            for instruction in instructions:
                instructionID = int(controller.getInputID(instruction))
                instructionValue = float(controller.getInputValue(instruction).strip())
                
                #Checks if the input is valid to send through serial
                if instructionID == LSTICK_TURN or instructionID in [LEFT_TRIGGER,RIGHT_TRIGGER]:
                    
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

def send_video(arduino):
    cap = cv2.VideoCapture(0)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(5)

        try:
            s.connect((SERVER_IP, PORT))
            connected = True
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}|Connected to server.")
        except socket.timeout as e:
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}| Connection attempt timed out")
            stop_threads.set()
        except OSError as e:
            logger.info(f"|{datetime.now().strftime('%H:%M:%S')}| Failed to connect to server")
            stop_threads.set()

        
        previous_msg = None
        while not stop_threads.is_set() and connected:
            ret, frame = cap.read()
            if not ret:
                break

            # Encode frame as JPEG
            _, buffer = cv2.imencode('.jpg', frame)
            frame_bytes = buffer.tobytes()

            # Send frame size then data
            msg = struct.pack('>I', len(frame_bytes)) + frame_bytes
            
            try:
                s.sendall(msg)

                # Wait for acknowledgment
                instructions = s.recv(1050).decode().split('|')

                for instruction in instructions:
                    print(instruction)

                    if previous_msg != instruction:
                        arduino.write(instruction.encode('utf-8'))
                        previous_msg = instruction
            except OSError as e:
                logger.info(f"|{datetime.now().strftime('%H:%M:%S')}| Communication to server timed out")
                stop_threads.set()

        cap.release()

def serial_interface(arduino):
    while True:
        instruction = thread_pipe.get()
        if instruction is not None:
            arduino.write(instruction)





def auto(controller, arduino):
    model_thread = threading.Thread(target=send_video,args=[arduino])
    stop_threads.clear()
    model_thread.start()
    
    while not stop_threads.is_set():
        for instruction in controller.getControllerInput():
            inputID = int(controller.getInputID(instruction))
            
        
            if inputID == neutral_mode:
                stop_threads.set()
                model_thread.join()
                #time.sleep(.5)
                arduino.write(instruction.encode('utf-8'))

        time.sleep(.2)

