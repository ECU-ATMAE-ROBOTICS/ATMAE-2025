import math

def determine_direction(img, colors, boxes, reference,threshold):
    """
    Determine the direction a robot should move to align with a target bin
    based on YOLO-detected bounding boxes and tape colors.

    Args:
        img (numpy.ndarray): Image from the camera (BGR format).
        colors (list): List of detected tape colors associated with bounding boxes.
        boxes (list): List of bounding box coordinates in the format [x0, y0, x1, y1].
        reference (str): The target color of the box to align with.
        threshold (int): Threshold value representing the tolerance around the 
            middle section of the image for alignment.

    Returns:
        str: A movement command string formatted as:
            - "9:-1\n" if the bot is close enough and aligned with the target bin.
            - "5:{amount_to_turn}\n" if the bot needs to turn, where `amount_to_turn`
              is a float between -1.0 and 1.0.
    """
    #Get center of image
    img_height, img_width = img.shape[:2]
    img_center = [img_height//2, img_width//2]
    target_bin = None


    try:
        #Get bin to align with
        target_bin = boxes[colors.index(reference)]
    
    #If the target box is not in view, go in the direction of detected boxes
    except:
        target_bin = boxes[0]
        
        

    bin_height, bin_width = [target_bin[3]-target_bin[1], target_bin[2]-target_bin[0]]
    bin_center = [target_bin[3]-bin_height//2, target_bin[2]-bin_width//2]

    #Get percent box takes up in image
    image_per = img_height*2 + img_width*2
    bin_per = bin_height*2 + bin_width*2
    
    percent_covered = (bin_per/image_per) * 100

    #print("image perimeter: ",image_per)
    #print("box perimeter: ", bin_per)

    #print("percent covered: ", percent_covered)

    #Stop the bot if it's close enough and aligned
    if percent_covered > 40 and bin_center[1] < img_center[1]+60 and bin_center[1] > img_center[1]-60:
        return "9:-1\n"
    
    #Siulate controller turn input based on how far from center bin is
    else:
        amount_to_turn = (bin_center[1]/img_center[1] -1) * .65
        return f"5:{amount_to_turn:.2f}\n"



