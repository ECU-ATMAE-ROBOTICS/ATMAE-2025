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
    "AeEnable": False,             # disable auto exposure
    "AwbEnable": False,            # disable auto white balance
    "ExposureTime": metadata["ExposureTime"],
    "AnalogueGain": metadata["AnalogueGain"],
    "ColourGains": metadata["ColourGains"]
})


# OpenCV window loop
while True:
    # Capture a frame as a numpy array
    frame = picam2.capture_array()
        
    # Find the average pixel color BEFORE conversion
    average = cv2.mean(frame)
    average = np.array([[average]], dtype=np.uint8)
    average = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    average = np.array(average, dtype=np.uint8)
    
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    
    # DEBUG - Run color detection
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    

    color_ranges = {
    'red': [
        ((0, 190, 50), (15, 255, 255)),      # Lower red
        ((140, 190, 50), (180, 255, 255))    # Upper red
    ],
    'green': [((35, 15, 30), (96, 255, 255))],
    'blue':  [((93, 200, 15), (125, 255, 255))],
    'yellow':[((16, 130, 70), (40, 255, 255))],
    }

    # Store pixel counts for each color
    color_pixel_counts = {}

    #Amount of pixels in image
    pixel_count = frame.shape[0] * frame.shape[1]
    
    value = 0

    # Generate masks and count non-zero pixels
    # OLD METHOD
    for color, ranges in color_ranges.items():
        mask = None
        for lower, upper in ranges:
            lower_np = np.array(lower, dtype=np.uint8)
            upper_np = np.array(upper, dtype=np.uint8)
            current_mask = cv2.inRange(hsv, lower_np, upper_np)
            mask = current_mask if mask is None else cv2.bitwise_or(mask, current_mask)
    
        count = cv2.countNonZero(mask)
        color_pixel_counts[color] = (count, mask)

    # Find the most likely tape color
    detected_color = max(color_pixel_counts, key=lambda c: color_pixel_counts[c][0])
        
    # NEW METHOD
     # for color, ranges in color_ranges.items():
         # for group in ranges:
            # if(cv2.inRange(average, group[0], group[1])):
               # detected_color = color
                 # break 

    # Show it with OpenCV
   # cv2.putText(frame, f"{detected_color}, Red: {int(average[0])} Green: {int(average[1])} Blue: {int(average[2])}", (10, 30),
     #           cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 200, 200), 2)
   # cv2.imshow("Pi Camera Module 2 - Preview", frame)

    # Exit if 'q' is pressed
   # if cv2.waitKey(1) & 0xFF == ord('q'):
     #   break

# Clean up
cv2.destroyAllWindows()
picam2.stop()

