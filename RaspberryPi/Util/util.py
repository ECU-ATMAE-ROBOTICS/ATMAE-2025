#Axis that are registered as valid input
validSticks = [5]
validTriggers = [9, 10]

#Keeps track of previous inputs sent by an ID to prevent serial clogging
prevInstructions = {5:0.0, 9:0.0, 10:0.0}