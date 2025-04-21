import cv2
import time
from PIL import Image, ImageDraw
from ultralytics import YOLO

model = YOLO('model/yolo11n_edgetpu.tflite')
cam = cv2.VideoCapture(0)
#ref, frame = cam.read()
frame_count = 0
while True:
    ref, frame = cam.read()
    try:
        x0,y0,x1,y1 = model.predict(source=frame)[0].boxes.xyxy[0].tolist()
        #frame = cv2.rectangle(frame, (x0,y1), (x1, y0), color=(0,0,255), thickness=2)
        print(x0, y0, x1, y1)
    except:
        print("no boxes")
        pass
    frame_count += 1
    #cv2.imshow("frame", frame)
