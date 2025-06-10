import cv2
from ultralytics import YOLO

model = YOLO('model/yolov8_edgetpu.tflite', task='detect')
img = cv2.imread('test_images/boxes.jpg')
img = cv2.resize(img, (420,420))
print(img.shape)
results = model.predict(img)[0]

print(len(results.boxes.xyxy))
for box in results.boxes.xyxy:
    box = box.int().tolist()
    print(box)

    cv2.rectangle(img, box[:2], box[2:], (0,0,255), 2)
    cv2.imwrite('yolo_output.jpg', img)




#img.show()
