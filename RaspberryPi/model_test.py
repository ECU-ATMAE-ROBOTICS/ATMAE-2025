from ultralytics import YOLO
from PIL import Image, ImageDraw

img_path = 'test_images/car.jpg'

model = YOLO("model/yolo11n_edgetpu.tflite", task='detect')
count = 0
while count < 5:
    img = Image.open(img_path)
    img = img.resize((200,200))
    results = model.predict(img)[0].boxes
    imgDraw = ImageDraw.Draw(img)
    imgDraw.rectangle(results.xyxy[0].numpy().tolist(), outline='red', width=1)
    print(results.xyxy[0].numpy().tolist())
    count += 1
    #img.show()
