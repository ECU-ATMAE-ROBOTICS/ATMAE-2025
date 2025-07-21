import cv2
from ultralytics import YOLO
import time
import colordetect
import direct


model = YOLO('model/best2_full_integer_quant_edgetpu.tflite', task='detect')
cam = cv2.VideoCapture(0)

if not cam.isOpened():
    print("Error: Could not open video.")
    exit()


direction = None
while True:
    boxes = []
    colors = []

    ref, frame = cam.read()

    if frame is None or not ref:
        continue
    
    #Determines the size of the middle section of frame
    frame_center = [frame.shape[0]//2, frame.shape[1]//2]
    threshold = 40
    

    if cv2.waitKey(1) and 0xFF == ord('q'): #or button == 'q':
        cv2.destroyAllWindows()
        cam.release()
        break
        
    try:
        #frame and crop
        frame = cv2.resize(frame, (420,420))
               
        # Get bboxes from model
        boxes= model.predict(source=cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), iou=.3)[0].boxes.xyxy.int().tolist()

        if len(boxes) > 0:
            # Get colors from bins and sort bins from left to right
            colors = colordetect.get_box_colors(frame, boxes)
            colors,boxes = colordetect.sort_bins(colors,boxes)
            
            # ----Colored bin to track ("blue") is hardcoded right now, will be automatically determined in the future-----
            direction = direct.determine_direction(frame, colors, boxes, "blue", 40)
           
           # Comment this out if you dont need visualization
            for indx, box in enumerate(boxes):
                cv2.rectangle(frame, box[:2], box[2:], (0,0,255), 2)
                frame = cv2.putText(frame, colors[indx], (box[0],box[1]), cv2.FONT_HERSHEY_SIMPLEX, .5, (0,0,0), 1)
        
    except Exception as e:
        print("Error:", e)
    
    # Comment this out if you dont need visualization
    if direction is not None:
        frame = cv2.putText(frame, direction, (40,20), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,255), 2)

    
    cv2.imshow('frame', frame) #comment this out if you dont need visualization
    time.sleep(.02)
