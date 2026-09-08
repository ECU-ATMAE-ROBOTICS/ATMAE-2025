# Server Module Documentation

## Overview

The server module (`RaspberryPi/server/`) runs on an external computer with a dedicated GPU and provides real-time YOLO-based object detection for autonomous robot control. It receives video frames from the robot's USB camera via TCP, processes them through a YOLOv11m model, and sends movement instructions back to the robot.

## Files

### `modelserver.py`
**Purpose:** Main server application that handles frame reception, YOLO inference, and movement instruction generation.

**Key Responsibilities:**
- Listens for TCP connections from the Raspberry Pi client on port 9999
- Receives video frames with binary protocol (4-byte length header + JPEG data)
- Runs YOLOv11m object detection to identify carton bins
- Detects bin colors using color analysis
- Determines movement direction and throttle based on detected bins
- Displays live video feed with detection visualization
- Sends movement instructions back to client

**Main Functions:**

#### `receive_frame(conn)`
Receives a single video frame from the client over TCP.

**Parameters:**
- `conn`: Socket connection object

**Returns:**
- `numpy.ndarray`: Decoded BGR image, or `None` if connection closed

**Protocol:**
1. Read 4-byte big-endian integer specifying frame size
2. Read frame data in chunks
3. Decode JPEG bytes to OpenCV image
4. Return BGR image

**Error Handling:**
- Returns `None` if connection closes or data is incomplete
- Handles partial packet reception gracefully

#### `start_server()`
Main server loop that processes frames and sends responses.

**Flow:**
1. Create TCP socket and bind to port 9999
2. Listen for incoming client connections
3. For each connection:
   - Receive frame via `receive_frame()`
   - Resize frame to 448×448
   - Run YOLO inference with IoU threshold of 0.3
   - If bins detected:
     - Extract bin colors using `colordetect.get_box_colors()`
     - Sort bins left-to-right using `colordetect.sort_bins()`
     - Determine turn direction using `direct.determine_direction()`
     - Determine forward/stop throttle using `direct.determine_throttle()`
     - Combine directions and throttle into response message
   - Display frame with bounding boxes and instructions (ESC to stop)
   - Send response message back to client
4. Handle connection closure and KeyboardInterrupt

**Response Format:**
```
"{direction}|{throttle}"
```
Example: `"5:-0.25\n|9:0.0\n"` (turn left, move forward)

**Configuration:**
```python
HOST = ''              # Listen on all interfaces
PORT = 9999
MODEL = YOLO(r'path/to/best.pt')  # YOLOv11m model weights
```

**Visualization:**
- Draws red bounding boxes around detected bins
- Overlays movement instructions on frame
- Marks alignment target (circle at 180,360)
- Displays in OpenCV window titled "Live Feed"
- Press ESC to stop visualization and close connection

**Error Handling:**
- Catches `KeyboardInterrupt` to gracefully shutdown
- Reconnects after client disconnection
- Prints connection status messages

### `colordetect.py`
**Purpose:** Provides color detection and bin analysis utilities for the server.

**Functions:**

#### `sort_bins(colors, boxes)`
Sorts YOLO-detected bounding boxes and their associated colors from left to right.

**Parameters:**
- `colors`: List of color labels (strings) for each box
- `boxes`: List of bounding box coordinates in YOLO xyxy format [x0, y0, x1, y1]

**Returns:**
- Tuple: `(sorted_colors, sorted_boxes)` ordered by x-coordinate (left to right)

**Implementation:**
- Uses bubble sort algorithm
- Compares `boxes[j][0]` (x-coordinate) to determine order
- Maintains correspondence between colors and boxes during sort

#### `detect_color(img)`
Detects the dominant color in an image using HSV color space thresholds.

**Parameters:**
- `img`: BGR format image (numpy array)

**Returns:**
- Color string: `"red"`, `"green"`, `"blue"`, `"yellow"`, or `"black"`

**Color Ranges (HSV):**
- **Red:** (0-10, 190-255, 50-255) and (140-180, 190-255, 50-255)
- **Green:** (41-93, 20-255, 5-255)
- **Blue:** (90-125, 200-255, 15-255)
- **Yellow:** (13-40, 130-255, 70-255)

**Detection Logic:**
1. Convert BGR image to HSV color space
2. Create binary masks for each color range
3. Count non-zero pixels for each color mask
4. Find color with highest pixel count
5. Return `"black"` if highest color covers <70% of image (threshold for distinguishing color vs background)
6. Apply mask and return detected color

**Note:** Different threshold (70%) than client version (40%) for more conservative bin detection

#### `get_box_colors(img, boxes)`
Extracts the dominant color within each detected bounding box.

**Parameters:**
- `img`: Full image (BGR)
- `boxes`: List of bounding box coordinates [x0, y0, x1, y1]

**Returns:**
- List of color strings corresponding to each box

**Implementation:**
- Crops image to each bounding box region
- Calls `detect_color()` on cropped region
- Returns list of detected colors in box order

### `direct.py`
**Purpose:** Calculates movement instructions based on detected bins and camera alignment.

**Functions:**

#### `clamp(n, smallest, largest)`
Utility function to constrain a value within a range.

**Parameters:**
- `n`: Value to constrain
- `smallest`: Minimum allowed value
- `largest`: Maximum allowed value

**Returns:**
- Clamped value between smallest and largest (inclusive)

#### `determine_direction(img, boxes, target_x_pos)`
Calculates turn direction to align robot with target bin.

**Parameters:**
- `img`: Camera image (BGR)
- `boxes`: Sorted list of bounding boxes [x0, y0, x1, y1]
- `target_x_pos`: X-coordinate target for alignment (typically 180 for 448×448 frame)

**Returns:**
- String command: `"5:{amount_to_turn}\n"`
  - `amount_to_turn`: Float between -1.0 and 1.0
  - Positive value: turn right
  - Negative value: turn left

**Logic:**
1. Get image dimensions and center
2. Use first (leftmost) bin as target
3. Calculate bin center: 
   - bin_center_x = (x0 + x1) / 2
4. Calculate turn amount:
   - `amount_to_turn = clamp(bin_center_x / target_x_pos - 1, -1, 1)`
5. Format as serial command with 2 decimal places

**Example:**
- Frame width: 448, target_x_pos: 180
- Bin center at x=180: `5:0.00\n` (no turn needed)
- Bin center at x=90: `5:-0.50\n` (turn left)
- Bin center at x=270: `5:0.50\n` (turn right)

#### `determine_throttle(img, boxes)`
Calculates forward/stop throttle based on bin size (distance to bin).

**Parameters:**
- `img`: Camera image (BGR)
- `boxes`: List of bounding boxes [x0, y0, x1, y1]

**Returns:**
- String command: `"9:-1\n"` (stop) or `"9:0\n"` (forward)

**Logic:**
1. Get image dimensions
2. Calculate first box dimensions:
   - bin_height = y1 - y0
   - bin_width = x1 - x0
3. Calculate percentage of image covered by bin:
   - `percent_covered = (bin_height * bin_width) / (img_height * img_width) * 100`
4. Return stop signal if bin covers >10% of frame
5. Otherwise return forward signal

**Rationale:**
- When bin is large (>10% of frame), robot is close enough to stop and collect
- When bin is small (<10% of frame), robot should continue driving toward it

### `model_cam_test.py`
**Purpose:** Standalone test script for model inference and movement calculation (development/testing utility).

**Functionality:**
- Captures frames from local USB camera (for testing on server machine)
- Runs YOLO inference similar to main server
- Tests color detection and direction calculation
- Displays results in OpenCV window
- Allows selection of target color to track

**Note:** Not used in production; main inference happens in `modelserver.py`

## Control Flow

```
Client sends video frame
    ↓
Server receives via TCP (receive_frame)
    ↓
Resize to 448×448
    ↓
YOLO Model Inference (iou=0.3)
    ↓
Bins detected?
    ├─ Yes:
    │   ├─ Extract colors (get_box_colors)
    │   ├─ Sort left-to-right (sort_bins)
    │   ├─ Calculate turn (determine_direction)
    │   ├─ Calculate throttle (determine_throttle)
    │   └─ Send: "{direction}|{throttle}"
    │
    └─ No:
        └─ Send: "None\n"
    ↓
Display frame with visualization
    ↓
Repeat
```

## Serial Command Protocol

Commands sent to client use format: `"ID:Value\n"`

**Movement Commands:**
- `5:{turn_amount}` - Turn amount (-1.0 to 1.0, negative=left, positive=right)
- `9:{throttle}` - Throttle (-1 to stop, 0 to move forward)

**Combined Format:** `"{direction}|{throttle}"` sent as single message

## Model Configuration

**YOLO Model:**
- Architecture: YOLOv11m (medium)
- Task: Object Detection
- Input size: 448×448
- Inference parameters:
  - IoU threshold: 0.3 (more inclusive)
  - Confidence: Default YOLO
- Output: Bounding boxes in xyxy format

**Model Path:**
- Development: `r'C:\Users\lozan\OneDrive\Documents\ATMAE\ATMAE-2025\RaspberryPi\server\best.pt'`
- Should be updated for production deployment

## Dependencies

- `socket` - TCP server communication
- `cv2` (OpenCV) - Image processing and display
- `numpy` - Numerical operations on images
- `struct` - Binary frame protocol
- `ultralytics` - YOLO model loading and inference
- `colordetect` - Custom color detection module
- `direct` - Custom movement calculation module

## Performance Considerations

- Resizes frames to 448×448 for faster inference
- IoU threshold of 0.3 for more detections (vs typical 0.5)
- Runs on GPU-enabled machine for real-time inference
- Sends movement commands continuously while receiving frames
- Visualization can be disabled by commenting out `cv2.imshow()` for faster processing

## Configuration & Tuning

**Frame Processing:**
- Frame resize: 448×448 (line 101)
- IoU threshold: 0.3 (line 103)

**Alignment:**
- Target X position: 180 (half of 448, for frame center - line 110)
- Alignment circle visualization: (180, 360)

**Distance Detection:**
- Bin size threshold: 10% of frame (line 111)

**Color Thresholds:**
- Adjust HSV ranges in `colordetect.py` for different lighting conditions
- Red: 2 ranges due to hue wrap-around
- Minimum coverage: 70% for server (40% for client)

## Troubleshooting

- **"Connection refused":** Check if server is running and listening on port 9999
- **"Failed to load model":** Verify YOLO model path and file exists
- **"No detections":** Check image quality, lighting, model training, IoU threshold
- **"Wrong colors detected":** Adjust HSV color ranges in `colordetect.py`
- **"Slow inference":** Verify GPU availability, check YOLO model size, reduce frame size
