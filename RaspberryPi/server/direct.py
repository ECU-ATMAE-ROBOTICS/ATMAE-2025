import math

def determine_direction(img, boxes):
    """
    Determine the direction a robot should move to align with a target bin
    based on YOLO-detected bounding boxes and tape colors.

    Args:
        img (numpy.ndarray): Image from the camera (BGR format).
        boxes (list): List of bounding box coordinates in the format [x0, y0, x1, y1].


    Returns:
        str: A movement command string formatted as:
            - "5:{amount_to_turn}\n" if the bot needs to turn, where `amount_to_turn`
              is a float between -1.0 and 1.0.
    """
    #Get center of image
    img_height, img_width = img.shape[:2]
    img_center = [img_height//2, img_width//2]
    target_bin = boxes[0]

    bin_height, bin_width = [target_bin[3]-target_bin[1], target_bin[2]-target_bin[0]]
    bin_center = [target_bin[3]-bin_height//2, target_bin[2]-bin_width//2]

    amount_to_turn = (bin_center[1]/img_center[1] -1)

    #Account for right drift
    if amount_to_turn > 0:
        amount_to_turn = 1

    turn = f"5:{amount_to_turn:.2f}\n" # Turn
    
    return turn

def determine_throttle(img, boxes):
    
    #Get center of image
    img_height, img_width = img.shape[:2]
    img_center = [img_height//2, img_width//2]
    target_bin = boxes[0]

    bin_height, bin_width = [target_bin[3]-target_bin[1], target_bin[2]-target_bin[0]]
    bin_center = [target_bin[3]-bin_height//2, target_bin[2]-bin_width//2]

    percent_covered =  ((bin_height*2 + bin_width*2)/(img_height*2 + img_width*2)) * 100 #percent of image covered by bin
    print(percent_covered)

    if percent_covered > 23:
        #Stop Signal
        return f"9:-1\n"
    else:
        #Forward Signal
        return f"9:-.6\n"





