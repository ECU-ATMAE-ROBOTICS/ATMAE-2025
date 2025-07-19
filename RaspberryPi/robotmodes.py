import logging
import time

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
        instructions = None
        instructions = controller.getControllerInput()

        #Checks if theres inputs from the controller
        if instructions != None:
        
            for instruction in instructions:
                instructionID = int(controller.getInputID(instruction))
                instructionValue = float(controller.getInputValue(instruction)[:-1])
                
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

def auto(controller, arduino):
    pass
