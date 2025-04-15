from ultralytics import YOLO

model = YOLO("yolo11s_edgetpu.tflite")
results = model.predict('car.jpg')[0].boxes

print(results)
