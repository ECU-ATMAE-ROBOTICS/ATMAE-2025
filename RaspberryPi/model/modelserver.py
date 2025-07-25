# server.py
import socket
import cv2
import numpy as np
import struct
from ultralytics import YOLO

import colordetect
import direct

HOST = ''  # Listen on all interfaces
PORT = 9999
MODEL = YOLO(r"RaspberryPi\model\best.pt")


def receive_frame(conn):
    # Receive 4-byte length header
    data = b''
    while len(data) < 4:
        packet = conn.recv(4 - len(data))
        if not packet:
            return None
        data += packet
    msg_len = struct.unpack('>I', data)[0]

    # Receive the actual frame
    data = b''
    while len(data) < msg_len:
        packet = conn.recv(msg_len - len(data))
        if not packet:
            return None
        data += packet

    # Decode JPEG bytes to image
    frame_data = np.frombuffer(data, dtype=np.uint8)
    frame = cv2.imdecode(frame_data, cv2.IMREAD_COLOR)
    return frame

def start_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen(1)
        print(f"[SERVER] Listening on port {PORT}...")
        conn, addr = s.accept()
        print("[SERVER] Connected by", addr)

        with conn:
            direction = None
            while True:
                frame = receive_frame(conn)
                if frame is None:
                    break

                # Display the received frame
                cv2.imshow("Live Feed", frame)

                # Get bboxes from model
                boxes= MODEL.predict(source=cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), iou=.3)[0].boxes.xyxy.int().tolist()

                if len(boxes) > 0:
                    # Get colors from bins and sort bins from left to right
                    colors = colordetect.get_box_colors(frame, boxes)
                    colors,boxes = colordetect.sort_bins(colors,boxes)
                    
                    # ----Colored bin to track ("blue") is hardcoded right now, will be automatically determined in the future-----
                    direction = direct.determine_direction(frame, colors, boxes, "blue", 40) + "\n"
                
                conn.sendall(direction.encode())

        cv2.destroyAllWindows()
        print("[SERVER] Connection closed.")

if __name__ == "__main__":
    while True:
        try:
            start_server()
        except ConnectionResetError:
            print("Client Connection closed")

