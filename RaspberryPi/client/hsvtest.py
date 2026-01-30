import cv2
import numpy as np

# Mouse callback function
def show_hsv(event, x, y, flags, param):
    if event == cv2.EVENT_MOUSEMOVE:  # When the mouse moves
        hsv_img = param  # HSV version of the image
        pixel = hsv_img[y, x]  # Get pixel HSV value
        h, s, v = pixel
        print(f"HSV at ({x}, {y}): H={h}, S={s}, V={v}")

        # Show HSV values on the window
        img_copy = img.copy()
        cv2.putText(img_copy, f"H:{h} S:{s} V:{v}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        cv2.imshow("Image", img_copy)

# Load image
img = cv2.imread("/home/ecu/Desktop/ATMAE-2025/RaspberryPi/client/Sorting System Screenshots/black/4.jpg")
if img is None:
    print("Error: Could not load image.")
    exit()

# Convert to HSV
hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

# Create window and set callback
cv2.imshow("Image", img)
cv2.setMouseCallback("Image", show_hsv, hsv)

# Wait until ESC is pressed
while True:
    if cv2.waitKey(1) & 0xFF == 27:  # ESC key
        break

cv2.destroyAllWindows()

