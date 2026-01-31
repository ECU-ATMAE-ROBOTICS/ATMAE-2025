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

minValues = np.array([255, 255, 255])
maxValues = np.array([0,0,0])


# OpenCV window loop
while True:
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
    
    
    
    #test color range IN BGR
    color_ranges = {
    'red': [((0, 0, 120), (209, 73, 255))],
    'blue':  [((100, 0, 0), (255, 100, 33))],
    'green': [((0, 50, 0), (233, 255, 100))],
    'yellow':[((0, 128, 115), (241, 255, 221))],
    }
    
    
    #Actual color range
    #color_ranges = {
    #'yellow':[((0, 130, 70), (70, 255, 255))],
    #'red': [
    #    ((0, 190, 50), (15, 255, 255)),      # Lower red
    #    ((140, 190, 50), (180, 255, 255))    # Upper red
    #],
    #'green': [((35, 15, 30), (96, 255, 255))],
    #'blue':  [((93, 200, 15), (125, 255, 255))],
    #}

    # Store pixel counts for each color
    color_pixel_counts = {}

    #Amount of pixels in image
    pixel_count = frame.shape[0] * frame.shape[1]
    
    value = 0
    


    # Generate masks and count non-zero pixels
    # OLD METHOD
    #for color, ranges in color_ranges.items():
    #    mask = None
    #    for lower, upper in ranges:
    #        lower_np = np.array(lower, dtype=np.uint8)
    #        upper_np = np.array(upper, dtype=np.uint8)
    #        current_mask = cv2.inRange(hsv, lower_np, upper_np)
    #        mask = current_mask if mask is None else cv2.bitwise_or(mask, current_mask)
    
    #    count = cv2.countNonZero(mask)
    #    color_pixel_counts[color] = (count, mask)

    # Find the most likely tape color
    #detected_color = max(color_pixel_counts, key=lambda c: color_pixel_counts[c][0])
    
    detected_color = None
    
    print("Test right before New Method")
    print(f"Average: {average}")
    average2 = average.tolist()
    #for i in range(3):
     #   if minValues[i] > average2[0][0][i]:
     #      minValues[i] = average2[0][0][i]
      #  if maxValues[i] < average2[0][0][i]:
       #     maxValues[i] = average2[0][0][i] 
     
    minValues = np.minimum(minValues, average[0][0])
    maxValues = np.maximum(maxValues, average[0][0])
    # NEW METHOD
    for color, ranges in color_ranges.items():
        if detected_color != None:
            break
        for group in ranges:
            if(cv2.inRange(average, group[0], group[1])):
                detected_color = color
                break 

    # Show it with OpenCV
    cv2.putText(frame, f"{detected_color}", (80, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 200, 200), (2))
    #cv2.putText(frame, f"Red: {int(average[0])} Green: {int(average[1])} Blue: {int(average[2])}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 200, 200), (2))
    
    cv2.imshow("Pi Camera Module 2 - Preview", frame)
    
    #print("After Show with OpenCV")
    #Exit if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
print(minValues)
print(maxValues)
    
# Clean up
cv2.destroyAllWindows()
picam2.stop()

