import io
import logging
import socketserver
from threading import Condition
from http.server import HTTPServer, BaseHTTPRequestHandler
import cv2
from picamera2 import Picamera2

#Host IP Address and camera initialization
HOST=   "0.0.0.0" 
capture = Picamera2()
capture.start()

#Web Page front-end

PORT = 9000
#PAGE="""\
#<html>
#<head>
#<title>Raspberry Pi - Surveillance Camera</title>
#</head>
#<body>
#<center><h1>Raspberry Pi - Surveillance Camera</h1></center>
#<center><img src="stream.mjpg" width="640" height="480"></center>
#</body>
#</html>
#"""


class StreamHTTP(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'multipart/x-mixed-replace; boundary=--jpgboundary')
        self.end_headers()

        #Stream the Camera to the webpage
        while True:
            img = capture.capture_array()
            img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB) 

            #Sending the image to the webpage
            r, buf = cv2.imencode(".jpg", img)
            self.wfile.write("--jpgboundary\r\n".encode())
            self.send_header('Content-type', 'image/jpeg')
            self.send_header('Content-length', str(len(buf)))
            self.end_headers()
            self.wfile.write(bytearray(buf))
            self.wfile.write('\r\n'.encode())

#Start the server to host the webpage
server = HTTPServer((HOST,PORT), StreamHTTP)
server.serve_forever()
server.server_close()

