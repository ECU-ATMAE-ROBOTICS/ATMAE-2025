import cv2
from ultralytics import YOLO
import time
import colordetect
import direct


model = YOLO('model/best2_full_integer_quant_edgetpu.tflite', task='detect')
cam = cv2.VideoCapture("test_images/vid2.mp4")

if not cam.isOpened():
    print("Error: Could not open video.")
    exit()

start= time.time()


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

        frame = cv2.resize(frame, (420,420))
        frame = frame[:, 100:]
        # Get image dimensions
        (h, w) = frame.shape[:2]
        center = (w // 2, h // 2)

        # Define rotation matrix: rotate 45 degrees counter-clockwise
        angle = -90
        scale = 1.0  # Keep original size
        rotation_matrix = cv2.getRotationMatrix2D(center, angle, scale)

        # Apply the rotation
        frame = cv2.warpAffine(frame, rotation_matrix, (w, h))

        boxes= model.predict(source=cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), iou=.3)[0].boxes.xyxy.int().tolist()

        if len(boxes) > 0:
            colors = colordetect.get_box_colors(frame, boxes)
            colors,boxes = colordetect.sort_bins(colors,boxes)

            try:
                direction = direct.determine_direction(frame, colors, boxes, "blue", 40)
            
            except Exception as e:
                print("Error with getting direction: ",e)

            for indx, box in enumerate(boxes):
                cv2.rectangle(frame, box[:2], box[2:], (0,0,255), 2)
                frame = cv2.putText(frame, colors[indx], (box[0],box[1]), cv2.FONT_HERSHEY_SIMPLEX, .5, (0,0,0), 1)



        
    except Exception as e:
        print("Error:", e)
    
    if direction is not None:
        frame = cv2.putText(frame, direction, (40,20), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,255), 2)

    print(direction)
    cv2.imshow('frame', frame)
    time.sleep(.02)
