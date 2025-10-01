import cv2
import numpy as np
import random

def sort_bins(colors, boxes):
    """
    Sort YOLO-detected bounding boxes and their associated colors
    from left to right (based on the x-coordinate of the box).

    Args:
        colors (list): List of color labels associated with each box.
        boxes (list): List of bounding box coordinates in the format [x0, y0, x1, y1].

    Returns:
        tuple: 
            - final_colors (list): Sorted list of colors from left to right.
            - final_boxes (list): Sorted list of bounding boxes from left to right.
    """
       
    final_colors = colors[:]
    final_boxes = boxes[:]

    #Bubble sort
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
    Detect the dominant tape color in an image using HSV color thresholds.

    Args:
        img (numpy.ndarray): Input BGR image

    Returns:
        str: The detected color label ('red', 'green', 'blue', 'yellow').
    """
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    

    color_ranges = {
    'red': [
        ((0, 190, 50), (10, 255, 255)),      # Lower red
        ((140, 190, 50), (180, 255, 255))    # Upper red
    ],
    'green': [((41, 20, 5), (93, 255, 255))],
    'blue':  [((90, 200, 15), (125, 255, 255))],
    'yellow':[((13, 130, 70), (40, 255, 255))],
    }

    # Store pixel counts for each color
    color_pixel_counts = {}

    #Amount of pixels in image
    pixel_count = img.shape[0] * img.shape[1]

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
    
    # Return black if the highest color doesnt take up 70% of the image
    if color_pixel_counts[detected_color][0]/pixel_count < .70:
        return "black"

    mask = color_pixel_counts[detected_color][1]
    result = cv2.bitwise_and(img, img, mask=mask)

    return detected_color



def get_box_colors(img,boxes):
    """
    Extracts bounding box regions from an image and determines the tape color
    inside each bounding box using detect_color().

    Args:
        img (numpy.ndarray): The original BGR image containing objects.
        boxes (list): List of bounding box coordinates in the format [x0, y0, x1, y1].

    Returns:
        list | None: 
            - List of detected colors for each bounding box.
            - None if no colors were detected.
    """
    colors = []
    
    for ind,box in enumerate(boxes):
        x0,y0,x1,y1 = box
        cropped_image = img[y0:y1, x0:x1]
        
        colors.append(detect_color(cropped_image))
        
    if len(colors) > 0:
        return colors
    else:
        return None


