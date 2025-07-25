# server.py
import socket
import cv2
import numpy as np
import struct

HOST = ''  # Listen on all interfaces
PORT = 9999

def receive_image(conn):
    # Receive 4 bytes first to know the length of the incoming image
    data = b''
    while len(data) < 4:
        data += conn.recv(4 - len(data))
    msg_len = struct.unpack('>I', data)[0]

    # Receive the actual image data
    data = b''
    while len(data) < msg_len:
        packet = conn.recv(msg_len - len(data))
        if not packet:
            return None
        data += packet

    # Decode image
    np_data = np.frombuffer(data, dtype=np.uint8)
    img = cv2.imdecode(np_data, cv2.IMREAD_COLOR)
    return img

def start_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen(1)
        print(f"Server listening on port {PORT}...")
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            img = receive_image(conn)

            if img is not None:
                cv2.imshow('Received Image', img)
                cv2.waitKey(0)
                cv2.destroyAllWindows()
                conn.sendall(b"Image received successfully.")
            else:
                print("Failed to receive image.")

if __name__ == "__main__":
    start_server()

