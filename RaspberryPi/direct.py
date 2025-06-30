import math

def determine_direction(img, colors, boxes, reference,threshold):
   
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

    print("image perimeter: ",image_per)
    print("box perimeter: ", bin_per)

    print("percent covered: ", percent_covered)

    #Stop the bot if it's close enough and aligned
    if percent_covered > 40 and bin_center[1] < img_center[1]+60 and bin_center[1] > img_center[1]-60:
        return "stop"
    
    if bin_center[1] > img_center[1]+threshold:
        return "right"
    elif bin_center[1] < img_center[1]+threshold and bin_center[1] > img_center[1]-threshold:
        return "forward"
    else:
        return "left"



