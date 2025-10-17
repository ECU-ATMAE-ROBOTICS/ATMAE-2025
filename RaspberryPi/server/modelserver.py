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

BOX_MODEL = r'RaspberryPi\server\best.pt'
CARTON_MODEL = r'tbd'
MODEL = YOLO(r'C:\Users\lozan\OneDrive\Documents\ATMAE\ATMAE-2025\RaspberryPi\server\best.pt')

def receive_frame(conn):

    """
    Receive a single video frame from a client over a TCP connection.

    The function first reads a 4-byte header specifying the length of the frame,
    then reads the JPEG-encoded frame data, and finally decodes it into
    an OpenCV BGR image.

    Args:
        conn (socket.socket): The socket connection object with the client.

    Returns:
        numpy.ndarray | None:
            - Decoded image frame in BGR format if successful.
            - None if the connection is closed or data is incomplete.
    """

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
    
    """
    Start the server to receive video frames, run YOLO-based object detection,
    determine movement direction, and send responses back to the client.

    The server:
        - Listens for incoming TCP connections.
        - Receives image frames from the client.
        - Runs YOLO model inference to detect bounding boxes.
        - Identifies bin colors using `colordetect`.
        - Sorts bins and determines movement direction using `direct`.
        - Displays the live video feed with bounding boxes and direction overlay.
        - Sends movement instructions back to the client.

    Args:
        None

    Returns:
        None
    """

    while True:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.bind((HOST, PORT))
                s.listen(100)
                print(f"[SERVER] Listening on port {PORT}...")
                conn, addr = s.accept()
                print("[SERVER] Connected by", addr)

                with conn:
                    
                    while True:
                        frame = receive_frame(conn)
                        messages = "None\n"

                        if frame is None:
                            break
                        #frame = cv2.resize(frame, (448,448))
                        # Get bboxes from model
                        boxes = MODEL.predict(source=cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), iou=.3, verbose=False)[0].boxes.xyxy.int().tolist()

                        if len(boxes) > 0:
                            # Get colors from bins and sort bins from left to right
                            colors = colordetect.get_box_colors(frame, boxes)
                            colors,boxes = colordetect.sort_bins(colors,boxes)
                            
                            direction = direct.determine_direction(frame, boxes)
                            throttle = direct.determine_throttle(frame, boxes)

                            if throttle == "9:-1\n":
                                print("switching to bin model")
                                pass

                            messages = f"{direction}|{throttle}"
                            print(messages)
                        
                        # Comment this out if you dont need visualization
                            for indx, box in enumerate(boxes):
                                cv2.rectangle(frame, box[:2], box[2:], (0,0,255), 2)
                            
                            frame = cv2.putText(frame, messages, (20,60), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,255), 2)
                        

                        # Display the received frame
                        cv2.imshow("Live Feed", frame)
                        if cv2.waitKey(1) == 27:  # Press 'ESC' to stop
                            break
                        
                        #print(fr"{messages}")
                        
                        conn.sendall(messages.encode())
                        messages = ''
                cv2.destroyAllWindows()
                print("[SERVER] Connection closed.")
                print("switching to carton model...\n")
            except KeyboardInterrupt:
                s.close()
                exit()


if __name__ == "__main__":
    MODEL.predict(r'RaspberryPi\server\warmup_image.png')
    start_server()
