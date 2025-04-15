from picamera2 import Picamera2
import time

picam = Picamera2(0)
picam.start()
time.sleep(2)
picam.capture_file("test.jpg")
