from picamera2 import Picamera2
import cv2
import time
import numpy as np

# Create Picamera2 object
picam2 = Picamera2(0)

# Configure the camera
preview_config = picam2.create_preview_configuration()
picam2.configure(preview_config)

time.sleep(2)

# Start the camera
picam2.start()

# Lock the exposure and gain
metadata = picam2.capture_metadata()

picam2.set_controls({
    "AeEnable": False, # disable auto exposure
    "ColourGains": (2.5,1.3),             
    "AwbEnable": False,            # disable auto white balance
    "ExposureTime":10000,
    "AnalogueGain": 1.0 - 4.0,
    "ExposureTime": metadata["ExposureTime"],
    "AnalogueGain": metadata["AnalogueGain"],
    "ColourGains": metadata["ColourGains"]
})

ticks = 500

colors = []

highestb = 0
highestg = 0
highestr = 0

lowestb = 255
lowestg = 255
lowestr = 255

# OpenCV window loop
while ticks > 0:
    # Capture a frame as a numpy array
    frame = picam2.capture_array()
        
    # Find the average pixel color BEFORE conversion
    average = cv2.mean(frame)
    average = np.array([[average]], dtype=np.uint8)
    average = cv2.cvtColor(average, cv2.COLOR_RGB2BGR)
    cv2.imshow("Average", average)
    #print(average)
    
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    
    # DEBUG - Run color detection
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Store pixel counts for each color
    
    #print(f"Average: {average}")
    colors.append(average)
    
    #HIGH BLUE
    if average[0][0][0] > highestb:
        highestb = average[0][0][0]
    
    #HIGH GREEN
    if average[0][0][1] > highestg:
        highestg = average[0][0][1]
    
    #HIGH RED
    if average[0][0][2] > highestr:
        highestr = average[0][0][2]
        
    
    
    #LOW BLUE
    if average[0][0][0] < lowestb:
        lowestb = average[0][0][0]
    
    #LOW GREEN
    if average[0][0][1] < lowestg:
        lowestg = average[0][0][1]
    
    #LOW RED
    if average[0][0][2] < lowestr:
        lowestr = average[0][0][2]
        
    print(f"LOW THRESH: ({lowestb}, {lowestg}, {lowestr})")
    print(f"HIGH THRESH: ({highestb}, {highestg}, {highestr})")
    
    detected_color = None
    

    # Show it with OpenCV
    cv2.putText(frame, f"{detected_color}", (80, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 200, 200), (2))
    #cv2.putText(frame, f"Red: {int(average[0])} Green: {int(average[1])} Blue: {int(average[2])}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 200, 200), (2))
    
    cv2.imshow("Pi Camera Module 2 - Preview", frame)
    
    ticks -= 1
    
    #print("After Show with OpenCV")
    #Exit if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
        
print(f"LOW THRESH: ({lowestb}, {lowestg}, {lowestr})")
print(f"HIGH THRESH: ({highestb}, {highestg}, {highestr})")
    
# Clean up
cv2.destroyAllWindows()
picam2.stop()

