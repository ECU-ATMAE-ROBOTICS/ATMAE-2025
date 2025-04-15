import serial
import pygame


class XboxController():
    """Class used to interface with an XboxController."""

    def __init__(self, controllerNum: int = 0, deadZone: float = 0):
        """
            Initialize an Xbox Controller

            Args:
                controllerNum (int) [default = 0]: Index to be used by pygame.joystick.Joystick().
                deadZone (float) [default = 0]: Distance the stick can move from neutral position before registering input.
        """
        pygame.init()

        self.controller = pygame.joystick.Joystick(controllerNum)
        self.deadZone = deadZone

        self.controllerName = self.controller.get_name()
        self.numAxis = self.controller.get_numaxes()
        self.numButtons = self.controller.get_numbuttons()
        self.numDpadButtons = 5
        self.total = self.numAxis + self.numButtons + self.numDpadButtons

        self.inputIDs = {"D": [id for id in range(self.numDpadButtons)],
                         "A": [id for id in range(self.numDpadButtons, self.total-self.numButtons)],
                         "B": [id for id in range(self.total-self.numButtons, self.total)]}

    def getLayout(self):
        """
    Prints the layout of the detected buttons on the XboxController
    """
        for buttonType in self.inputIDs.keys():
            print(f"{buttonType}: {self.inputIDs.get(buttonType)}")

    def getRawButton(self, buttonID: int) -> int:
        """
    Returns the value of a button
    Args:
        ButtonID (int): The id of the button
    Return:
        int: Value of the button pressed
    """
        return self.controller.get_button(buttonID)

    def getRawAxis(self, axisID: int) -> float:
        """
    Returns the value of a axis
    Args:
        axisID (int): The id of the axis
    Return:
        int: value of the axis
    """
        return self.controller.get_axis(axisID)

    def getRawdPad(self, dPadID: int) -> tuple[int, int]:
        """
    Returns the value of a axis
    Args:
        d-padID (int): The id of the d-pad
    Returns:
        integer tuple: A tuple that is in the format (0,0) where the x and y are represent which
                        side of the D-Pad 
    """
        return self.controller.get_hat(dPadID)

    async def getControllerInput(self):
        """
        Retrieves the value of any new inputs from the value. The function only detects input if its value
    has changed.

    Returns:
        string list: A list containing multiple values from multiple buttons/triggers/joysticks
                     from the controller in the format: [Input ID]:[Input Value].

    """
        message = []
        for event in pygame.event.get():
            if event.type == pygame.JOYBUTTONUP or event.type == pygame.JOYBUTTONDOWN:
                dictVal = self.inputIDs.get("B")
                value = f"{dictVal[event.button]}:{self.controller.get_button(event.button)}\n"
                message.append(value)

            if event.type == pygame.JOYAXISMOTION:
                axisValue = round(self.controller.get_axis(event.axis), 1)
                dictVal = self.inputIDs.get("A")

                if axisValue > self.deadZone or axisValue < self.deadZone*-1 or event.axis == 4 or event.axis == 5:
                    value = f"{dictVal[event.axis]}:{axisValue}\n"
                    message.append(value)
                else:
                    value = f"{dictVal[event.axis]}:0\n"
                    message.append(value)

            if event.type == pygame.JOYHATMOTION:
                dictVal = self.inputIDs.get("D")
                dPadVal = self.controller.get_hat(event.hat)
                if dPadVal == (0, 0):
                    value = f"{dictVal[0]}:{0}\n"
                    message.append(value)
                elif dPadVal == (0, 1):
                    value = f"{dictVal[1]}:{1}\n"
                    message.append(value)

                elif dPadVal == (1, 0):
                    value = f"{dictVal[2]}:{2}\n"
                    message.append(value)

                elif dPadVal == (0, -1):
                    value = f"{dictVal[3]}:{3}\n"
                    message.append(value)

                elif dPadVal == (-1, 0):
                    value = f"{dictVal[4]}:{4}\n"
                    message.append(value)

        return message
