# Client Module Documentation

## Overview

The client module (`RaspberryPi/client/`) runs on the Raspberry Pi and handles the robot's control logic. It communicates with the Xbox controller for teleoperation, manages camera input, sends video feeds to the server for autonomous processing, and controls the Arduino via serial communication.

## Files

### `main.py`
**Purpose:** Entry point for the robot control system.

**Key Responsibilities:**
- Initializes Xbox controller connection via pygame
- Establishes serial connection to the Arduino
- Manages operator-selected color for the sorting competition
- Routes control between teleoperation and autonomous modes based on controller input
- Provides logging for diagnostics

**Key Features:**
- Automatic reconnection to controller and Arduino if connection fails
- Serial communication testing on startup
- Color selection via Xbox buttons (A=Green, B=Red, X=Blue, Y=Yellow)
- Mode selection and control flow

### `robotmodes.py`
**Purpose:** Implements the three main operational modes for the robot.

**Functions:**

#### `teleop(controller, arduino)`
Runs the robot in **teleoperation mode** using Xbox controller inputs.
- Reads continuous controller input
- Maps controller axes to robot movement:
  - Left stick (X) → turning (ID 5)
  - Right trigger → forward movement (ID 10)
  - Left trigger → reverse movement (ID 9)
  - Right stick → arm/clamp control (IDs 7, 8)
- Maps buttons (A/B/X/Y) to gate control for color bins
- Prevents redundant serial commands to avoid congestion
- Returns when neutral mode (button 23) is pressed

#### `auto(controller, arduino)`
Runs the robot in **autonomous mode** with server-guided movement.
- Sends initial movement commands (forward, open paddles/clamp)
- Spawns `send_video()` thread to stream camera feed to server
- Receives movement instructions from server and relays to Arduino
- Monitors for neutral mode button to stop autonomous operation
- Coordinates with server for bin approach and object collection

#### `send_video(arduino)`
Handles **video streaming and server communication** during autonomous mode.
- Captures frames from USB camera (OpenCV)
- Encodes frames as JPEG and sends to server over TCP (port 9999)
- Receives movement instructions from server
- Relays instructions to Arduino via serial
- Detects when robot is close to cartons (throttle = -1)
- Triggers screw descent, clamp closure, and internal sorting sequence
- Manages thread coordination with `internal_sort_mode()`

#### `internal_sort_mode(arduino)`
Runs the **internal color detection and sorting** routine.
- Captures frames using PiCamera2 module
- Disables auto-exposure and white balance for consistent color detection
- Uses `colordetect.detect_color()` to identify ball colors (red/green/blue/yellow)
- Sends appropriate servo commands to Arduino to route balls to color bins
- Continues until 12 balls are sorted or 5 consecutive "no ball" detections occur
- Manages airlock servo sequence for ball delivery

### `colordetect.py`
**Purpose:** Provides color detection utilities for ball sorting.

**Functions:**

#### `detect_color(img)`
Detects the dominant color in an image using HSV color thresholds.

**Parameters:**
- `img`: BGR format image (numpy array)

**Returns:**
- Color string: `"red"`, `"green"`, `"blue"`, `"yellow"`, or `"black"` (if no dominant color)

**Color Ranges (HSV):**
- **Red:** (0-15, 190-255, 50-255) and (140-180, 190-255, 50-255)
- **Green:** (35-96, 15-255, 30-255)
- **Blue:** (93-125, 200-255, 15-255)
- **Yellow:** (16-40, 130-255, 70-255)

**Detection Logic:**
- Converts image to HSV color space
- Creates masks for each color range
- Returns color with highest pixel count
- Returns `"black"` if highest color covers <40% of image

#### `sort_bins(colors, boxes)`
Sorts YOLO-detected bounding boxes and their colors from left to right.

**Parameters:**
- `colors`: List of color labels for each box
- `boxes`: List of bounding box coordinates [x0, y0, x1, y1]

**Returns:**
- Tuple of (sorted_colors, sorted_boxes) ordered by x-coordinate

**Implementation:** Bubble sort based on box x-position

### `internalsort.py`
**Purpose:** Legacy internal sorting module (appears to be superseded by functionality in `robotmodes.py`).

**Function:**
- `internal_sort(frame)`: Returns color of detected ball or fallback value

## Control Flow

### Teleoperation Mode
```
main.py → Xbox Input → robotmodes.teleop() → Serial to Arduino → Motor Control
```

### Autonomous Mode
```
main.py → robotmodes.auto() → robotmodes.send_video()
    ↓
Capture Frame → Server (TCP 9999) → YOLO Detection
    ↓
Server Returns: Movement Instructions
    ↓
Send to Arduino → Drive Towards Bin
    ↓
Robot Close? → internal_sort_mode() → Color Sort 12 Balls
```

## Serial Communication Protocol

Commands sent to Arduino are formatted as strings ending with `\n`:

**Axis-based commands (format: `ID:Value`):**
- `5:value` - Turn (value: -1.0 to 1.0)
- `9:value` - Forward/Reverse throttle
- `10:value` - Left trigger (reverse)

**Button commands:**
- `openGreen`, `closeGreen` - Green bin gate
- `openRed`, `closeRed` - Red bin gate
- `openBlue`, `closeBlue` - Blue bin gate
- `openYellow`, `closeYellow` - Yellow bin gate
- `toRed`, `toGreen`, `toBlue`, `toYellow` - Route ball to specific color bin
- `goForward`, `turnAround` - Autonomous movement
- `screwUp`, `screwDown` - Lift mechanism
- `openClamp`, `closeClamp` - Carton clamp
- `openTopAirLock`, `closeTopAirLock` - Top airlock servo
- `openBottomAirLock`, `closeBottomAirLock` - Bottom airlock servo

## Threading Model

The client uses threading for concurrent operations:
- **Main thread:** Controller input polling and mode switching
- **Video thread:** Spawned in autonomous mode for server communication
- **Internal sort thread:** Spawned when robot approaches cartons for ball sorting

**Thread synchronization uses:**
- `threading.Event()` objects for inter-thread signaling
- Example: `stop_video_thread`, `robot_is_close`, `sorting_started`, `bin_mode`

## Configuration

Key configuration constants:
- `SERVER_IP = '192.168.0.227'` - Server computer IP for autonomous mode
- `PORT = 9999` - TCP port for video streaming
- Controller button/axis IDs for Xbox controller mapping
- Color detection thresholds in HSV color space

## Dependencies

- `pygame` - Xbox controller input
- `opencv-python` - Video capture and processing
- `picamera2` - PiCamera2 module for ball color detection
- `logging` - Diagnostics and logging
- `socket` - TCP communication with server
- `threading` - Concurrent thread management
- `struct` - Binary frame encoding/decoding
