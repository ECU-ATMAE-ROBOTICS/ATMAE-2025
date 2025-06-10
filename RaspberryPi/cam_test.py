import cv2
from ultralytics import YOLO
import os
import time

os.environ['QT_QPA_PLATFORM'] = 'xcb'

model = YOLO('model/ecu_model_edgetpu.tflite', task='detect')
cam = cv2.VideoCapture(0)
pic_count = 0

start= time.time()
while True:

    ref, frame = cam.read()


    if cv2.waitKey(1) and 0xFF == ord('q'): #or button == 'q':
        cv2.destroyAllWindows()
        cam.release()
        break
    elif (time.time()-start) > 2 :
        
        try:
            
            boxes= model.predict(source=cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))[0].boxes.xyxy.int().tolist()
            print(boxes)        
            for box in boxes:
                cv2.rectangle(frame, box[:2], box[2:], (0,0,255), 2)
        
        except Exception as e:
            print("Error:", e)

        start = time.time()
        
        if len(boxes) > 0:
            cv2.imshow('frame', frame)
