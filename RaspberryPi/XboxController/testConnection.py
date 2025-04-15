import pygame
import time
import os
from platform import system
import logging


connected = False
while not connected:
    try:
        pygame.init()
        controller = pygame.joystick.Joystick(0)
        connected = True
    except pygame.error:
        print("Couldn't connect to controller")
        pygame.joystick.quit()
    time.sleep(1)

if (system() == "Windows"):
    os.system('cls')
else:
    os.system('clear')

axes = [axis for axis in range(controller.get_numaxes())]
buttons = [button for button in range(controller.get_numbuttons())]
dPads = [dPad for dPad in range(controller.get_numhats())]

#Line number of where the outputs start
axisStart = 6
buttonStart = axisStart + len(axes) + 1
dPadStart = buttonStart + len(buttons) + 1

print(f"connected to {controller.get_name()}")
print(f"the controller has {controller.get_numbuttons()} buttons")
print(f"the controller has {controller.get_numaxes()} axes")
print(f"the controller has {controller.get_numhats()} D-pad\n")

            
print(5*"-"+"Axis Values"+5*"-") 
for axis in axes:
    print(f"Axis {axis}: {controller.get_axis(axis)}")

print(5*"-"+"Button Values"+5*"-")
for button in buttons:

    print(f"Button {button}: {controller.get_button(button)}")


print(5*"-"+"D-Pad Values"+5*"-")
for dPad in dPads:
    print(f"D-pad {dPad}: {controller.get_hat(dPad)}")


# print(f"\033[{axisStart};0H")
while True:
    for event in pygame.event.get():

        if event.type == pygame.JOYBUTTONUP or event.type == pygame.JOYBUTTONDOWN:

             print(f"\033[{buttonStart};1H", end = "")
             print(f"\033[{(event.button + 1)}B", end = "")
             print(f"Button {event.button}: {controller.get_button(event.button)}")



        if event.type == pygame.JOYAXISMOTION:
             print(f"\033[{axisStart};1H",end="")
             print(f"\033[{event.axis + 1}B",end="")
             print("\033[K", end="")
             print(f"Axis {event.axis}: {controller.get_axis(event.axis)}")


        if event.type == pygame.JOYHATMOTION:
             print(f"\033[{dPadStart};1H", end="")
             print(f"\033[{event.hat + 1}B",end="")
             print("\033[K", end="")
             print(f"D-pad {event.hat}: {controller.get_hat(event.hat)}")






