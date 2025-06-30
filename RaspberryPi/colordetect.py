import cv2
import numpy as np
import random

def sort_bins(colors, boxes):
    final_colors = colors[:]
    final_boxes = boxes[:]

    #Bubble sort bby
    for i in range(len(final_colors)):
        for j in range(len(final_colors)-i-1):
            if final_boxes[j][0] > final_boxes[j+1][0]:
                temp_color = final_colors[j+1]
                temp_box = final_boxes[j+1]

                final_colors[j+1] = final_colors[j]
                final_colors[j] = temp_color
                final_boxes[j+1] = final_boxes[j]
                final_boxes[j] = temp_box
    
    return final_colors, final_boxes


                   


def detect_color(img):
    """
    Determines the most prevalent color in an image
    """
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    

    color_ranges = {
    'red': [
        ((0, 100, 100), (13, 255, 255)),      # Lower red
        ((160, 100, 100), (180, 255, 255))    # Upper red
    ],
    'green': [((35, 100, 30), (85, 255, 255))],
    'blue':  [((90, 100, 30), (130, 255, 255))],
    'yellow':[((20, 30, 30), (34, 255, 255))]
    }

    # Store pixel counts for each color
    color_pixel_counts = {}

    # Generate masks and count non-zero pixels
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
    mask = color_pixel_counts[detected_color][1]
    result = cv2.bitwise_and(img, img, mask=mask)

    #cv2.imwrite(f'/home/ecu/Desktop/ATMAE-2025/RaspberryPi/yolo_detections/object{random.randint(0,100)}.jpg', result)
    return detected_color

def validate_box_colors(stored_color, detected_colors):
    valid_colors = ['red', 'green', 'blue', 'yellow']

    #Remove used colors
    valid_colors.remove(stored_color)




#Crops image to get box color
def get_box_colors(img,boxes):
    colors = []
    
    for ind,box in enumerate(boxes):
        x0,y0,x1,y1 = box
        cropped_image = img[y0:y1, x0:x1]
        
        #Further cropping the image to focus on tape
        h,w = cropped_image.shape[:2]
        #cropped_image = cropped_image[int(.25*h):int(h-(.25*h)), int(.30*w):int(w-(.30*w))] 


        colors.append(detect_color(cropped_image))
        
    if len(colors) > 0:
        return colors
    else:
        return None


