# Arduino Module Documentation

## Overview

The Arduino module (`Arduino/ATMAE-2025/ATMAE-2025.ino`) runs on the Arduino Giga R1 WiFi and handles all low-level robot control. It receives serial commands from the Raspberry Pi, manages servo motors for movement and manipulation, handles limit switches for the lifting system, and controls the color-sorting distribution system.

## Hardware Configuration

### Motor Control Pins

| Component | Pin | Type | Purpose |
|-----------|-----|------|---------|
| Right Drive Motor | 10 | PWM Servo | Forward/Reverse movement |
| Left Drive Motor | 13 | PWM Servo | Forward/Reverse movement (inverted) |
| Screw Servo | 44 | PWM Servo | Lifting mechanism for cartons |

### Limit Switch Pins

| Switch | Pin | Type | Purpose |
|--------|-----|------|---------|
| Top Lift Limit | 52 | INPUT_PULLUP | Detects when screw reaches top position |
| Bottom Lift Limit | 39 | INPUT_PULLUP | Detects when screw reaches bottom position |
| Color Sort Limit 1 | 37 | INPUT_PULLUP | Position detection for color sorter |
| Color Sort Limit 2 | 31 | INPUT_PULLUP | Position detection for color sorter |
| Color Sort Limit 3 | 33 | INPUT_PULLUP | Position detection for color sorter |
| Color Sort Limit 4 | 35 | INPUT_PULLUP | Position detection for color sorter |

### Servo Control Pins

| Servo | Pin | Purpose | Open Position | Close Position |
|-------|-----|---------|----------------|-----------------|
| Right Paddle | 9 | Carton collection | 125 | 30 |
| Left Paddle | 11 | Carton collection | 85 | 0 |
| Clamp | 12 | Carton clamping | 180 | 0 |
| Color Sorter | 6 | Ball routing servo | Variable (40-140) | 90 (stop) |
| Top Airlock | 2 | Top delivery airlock | 90 | 180 |
| Bottom Airlock | 3 | Bottom delivery airlock | 180 | 90 |
| Left Pipe Gate | 5 | Color bin gate | 90 | 0 |
| Mid Pipe Gate | 7 | Color bin gate | 160 | 90 |
| Right Pipe Gate | 4 | Color bin gate | 180 | 90 |
| Sep Pipe Gate | 8 | Separation bin gate | 90 | 160 |

### Motor PWM Values

**ESC/Servo PWM Control (Microseconds):**
- **1500:** Stop/Neutral
- **1000:** Full reverse/up (screw)
- **2000:** Full forward/down (screw)
- **1700:** Forward movement
- **1200-1800:** Turn in place (left/right differential)

## Main Components

### Global Variables

**Motor Control:**
```cpp
double RightTrigger = 0;      // Forward throttle (0-1)
double LeftTrigger = 0;       // Reverse throttle (0-1)
double LeftStick = 0;         // Turn input (-1 to 1)
double drive = 0;             // Combined drive command
double leftTurn = 0;          // Left turn ratio
double rightTurn = 0;         // Right turn ratio
double LMotor = 1500;         // Left motor PWM value
double RMotor = 1500;         // Right motor PWM value
```

**State Flags:**
```cpp
bool Auto = false;            // Autonomous mode flag
bool clampOpen = false;       // Clamp state tracking
bool screwStopped = true;     // Screw motion state
bool screwOverRide = false;   // Manual screw control override
bool reachedTop = true;       // Screw top limit reached
bool reachedBottom = true;    // Screw bottom limit reached
bool innitPosRed = false;     // Red position initialization flag
bool innitPosYel = false;     // Yellow position initialization flag
int currentPin = 1;           // Current color sorter position
String colorPins[4];          // Array of color pin assignments
```

## Core Functions

### `setup()`
Runs once at startup to initialize all hardware.

**Initialization:**
1. Begin serial communication at 9600 baud
2. Attach all servo objects to their pins
3. Configure limit switches as INPUT_PULLUP
4. Initialize all servos to starting positions
5. Run color sorter calibration sequence:
   - Move to limit switch 2, then stop
   - Move to limit switch 4, then stop
6. Set paddles to closed position
7. Delay 2 seconds for stabilization

### `loop()`
Main control loop running continuously.

**Operations:**
1. Check for serial data and parse commands
2. Calculate drive command: `drive = RightTrigger - LeftTrigger`
3. Reset motor PWM to neutral (1500)
4. Check limit switches:
   - If top limit hit and `reachedTop` flag set: stop screw, clear flags
   - If bottom limit hit and `reachedBottom` flag set: stop screw, clear flags
5. Calculate motor speeds based on drive and turn inputs:
   - If driving: apply turn ratios to reduce motor speed
   - If turning in place: apply equal and opposite speeds
6. Write PWM values to motors

### `parseData(String data)`
Parses incoming serial commands and executes corresponding actions.

**Command Types:**

**1. Axis Commands (Format: `ID:Value`)**
Commands with colon separator are parsed as ID and floating-point value.

- **ID 9 (LEFT_TRIGGER_ID):** Reverse throttle
  - Converts from range [-1, 1] to [0, 1]
  - Values < 0.2 set to 0 (deadzone)
  
- **ID 10 (RIGHT_TRIGGER_ID):** Forward throttle
  - Converts from range [-1, 1] to [0, 1]
  - Values < 0.2 set to 0 (deadzone)

- **ID 5 (LEFT_STICK_ID):** Turning control
  - Positive values: turn right (increase rightTurn ratio)
  - Negative values: turn left (increase leftTurn ratio)
  - Zero: no turning

- **ID 7 (Right_STICK_IDX):** Clamp and paddle control
  - Positive: open clamp and paddles
  - Negative: close clamp
  - Opens paddles first

- **ID 8 (Right_STICK_IDY):** Screw vertical control
  - Positive: lower screw (screwDown)
  - Negative: raise screw (screwUp)

- **ID 23 (NEUTRAL_ID):** Neutral mode
  - Calls resetBot() to stop all motion and reset positions

**2. String Commands (No colon)**
Commands without colon are string-based control sequences.

**Mode Selection Commands:**
- `"sepRed"` / `"sepGreen"` / `"sepBlue"` / `"sepYellow"`: Set up color sorter for specific bin
  - Reassigns limit switches to color positions
  - Closes all gates
  - Initializes sorter position

**Color Routing Commands:**
- `"toRed"` / `"toGreen"` / `"toBlue"` / `"toYellow"`: Move sorter to color position and dispense ball
  - Moves sorter servo to target color position
  - Waits for limit switch activation
  - Opens and closes airlocks in sequence
  - Opens top airlock when done

**Gate Control Commands:**
- `"openRed"` / `"closeRed"` (same for Green, Blue, Yellow): Open/close bin gate
- `"openGreen"` / `"closeGreen"`
- `"openBlue"` / `"closeBlue"`
- `"openYellow"` / `"closeYellow"`

**Airlock Control:**
- `"openTopAirLock"` / `"closeTopAirLock"`: Top airlock servo
- `"openBottomAirLock"` / `"closeBottomAirLock"`: Bottom airlock servo

**Clamp Control:**
- `"openClamp"`: Open clamp (180°)
- `"closeClamp"`: Close clamp (0°)
- `"stopClamp"`: Neutral clamp position (1500µs)

**Screw Control:**
- `"screwUp"`: Raise screw (1000µs)
- `"screwDown"`: Lower screw (2000µs)
- `"screwStop"`: Neutral screw (1500µs)

**Movement Commands:**
- `"goForward"`: Drive forward for 2 seconds
- `"turnAround"`: Spin 180° for 2 seconds
- `"shake"`: Shake robot side-to-side (for dislodging stuck balls)

**Sorting Control:**
- `"stopSort"`: Stop color sorter (neutral position)

## Critical Functions

### `resetBot()`
Resets all robot systems to safe/neutral state.

**Operations:**
1. Reset all control variables to 0
2. Stop both drive motors (1500µs)
3. Stop screw servo (1500µs)
4. Close all airlocks and clamps
5. Close all paddle gates
6. Recalibrate color sorter:
   - Move to limit 2, stop
   - Move to limit 4, stop

**Usage:** Called when exiting autonomous mode or when neutral button pressed

### Color Routing Functions

#### `toRed()` / `toGreen()` / `toBlue()` / `toYellow()`
Routes a detected ball to its corresponding color bin.

**Generic Flow:**
1. Close top airlock
2. Determine movement direction to target color limit switch:
   - If target position < current: move sorter backward (speed 140)
   - If target position > current: move sorter forward (speed 40)
   - If already at target: set initialization flag
3. Loop until target limit switch is triggered (or initialization flag set)
4. Stop sorter servo (90°)
5. Close top airlock
6. Open bottom airlock (1 second)
7. Close bottom airlock (1 second)
8. Open top airlock
9. Break and return

### Airlock Sequences

#### `openTopAirLock()` / `closeTopAirLock()`
**Open:** 90° | **Close:** 180°

#### `openBottomAirLock()` / `closeBottomAirLock()`
**Open:** 180° | **Close:** 90°

**Typical Dispensing Sequence:**
```
closeTopAirLock()
delay(1000)
openBottomAirLock()
delay(1000)
closeBottomAirLock()
delay(1000)
openTopAirLock()
```

### Paddle and Clamp Control

#### `openPaddles()` / `closePaddles()`
Controls dual servo paddles for carton collection.

**Open Positions:**
- Left Paddle: 85°
- Right Paddle: 125°

**Close Positions:**
- Left Paddle: 0°
- Right Paddle: 30°

#### `openClamp()` / `closeClamp()`
Controls clamp servo for holding cartons.

**Open:** 180° | **Close:** 0°

Includes state flag to prevent redundant operations.

### Movement Functions

#### `goForward()`
Drives robot forward for 2 seconds.
```cpp
1700µs (forward) → delay 2s → 1500µs (stop)
```

#### `turnAround()`
Spins robot 180° for 2 seconds.
```cpp
Left: 1200µs (reverse), Right: 1800µs (forward)
delay 2s → both 1500µs (stop)
```

#### `shake()`
Oscillates robot side-to-side to dislodge stuck balls.
```cpp
1300µs (slow left) → 500ms
1700µs (slow right) → 500ms
1500µs (stop)
```

### Screw Control

#### `screwUp()`
Raises screw servo at full speed until top limit reached.
- Speed: 1000µs
- Sets `reachedTop = true` flag for limit detection
- Delays 1 second before flag reset

#### `screwDown()`
Lowers screw servo at full speed until bottom limit reached.
- Speed: 2000µs
- Sets `reachedBottom = true` flag for limit detection
- Delays 1 second before flag reset

### Utility Functions

#### `openBottom(int pin)`
Generic function to open a bin gate based on pin assignment.

**Pin Mappings:**
- `limitSwitchPinOne`: sepPipeGate → 90°
- `limitSwitchPinTwo`: rightPipeGate → 90°
- `limitSwitchPinThree`: midPipeGate → 160°
- `limitSwitchPinFour`: leftPipeGate → 90°

#### `closeBottom(int pin)`
Generic function to close a bin gate based on pin assignment.

**Pin Mappings:**
- `limitSwitchPinOne`: sepPipeGate → 160°
- `limitSwitchPinTwo`: rightPipeGate → 175°
- `limitSwitchPinThree`: midPipeGate → 75°
- `limitSwitchPinFour`: leftPipeGate → 0°

#### `stopSort()`
Stops color sorter servo at neutral position (90°).

#### `indexfromkey(String key)`
Finds index of color in `colorPins[]` array.

**Parameters:**
- `key`: Color string (e.g., "redPin", "greenPin")

**Returns:**
- Array index (0-3) if found, -1 if not found

**Usage:** Maps color names to sorter positions dynamically

## Serial Communication Protocol

### Baud Rate
**9600 baud**

### Command Format

**Axis Commands (with PWM values):**
```
ID:Value\n
5:0.75\n          // Turn right
9:0.5\n           // Forward at 50%
10:-0.25\n        // Reverse at 25%
```

**String Commands:**
```
toRed\n
openClamp\n
screwDown\n
goForward\n
```

### Handshake
On startup, Raspberry Pi repeatedly sends `"Testing\n"` until Arduino responds, confirming serial connection is working.

## State Machine Overview

```
┌─────────────────┐
│  Loop Running   │
└────────┬────────┘
         │
    ┌────▼─────────────────────────┐
    │ Check Serial for Commands    │
    └────┬───────────────────┬─────┘
         │                   │
    [Axis Command]      [String Command]
         │                   │
    ├─Throttle          ├─Movement (goForward, shake)
    ├─Turn              ├─Color Routing (toRed, etc)
    ├─Clamp/Paddle      ├─Gate Control (openRed, etc)
    └─Screw             ├─Airlock Control
                        └─Sorting Control
         │                   │
         └────────┬──────────┘
                  │
         ┌────────▼────────────┐
         │ Apply PWM to Motors │
         │ Check Limit Switches│
         └─────────────────────┘
```

## Limit Switch Logic

**Top Limit Switch (Pin 52):**
- Triggered when screw reaches maximum height
- LOW = pressed, HIGH = released
- Stops screw motion when triggered with `reachedTop` flag

**Bottom Limit Switch (Pin 39):**
- Triggered when screw reaches minimum height
- LOW = pressed, HIGH = released
- Stops screw motion when triggered with `reachedBottom` flag

**Color Position Limits (Pins 37, 31, 33, 35):**
- Used to detect when color sorter reaches calibration positions
- Used to detect when sorter reaches target color position
- Polling occurs in color routing functions (e.g., `toRed()`)

## Operational Modes

### Teleoperation Mode
1. Read Xbox controller input from Pi
2. Update motor speeds based on trigger and stick values
3. Apply turn ratios for steering while driving
4. Manual clamp/paddle/screw control via right stick
5. Gate control via Xbox button mapping

### Autonomous Mode
1. Receive `"goForward"` command to approach cartons
2. Pi sends continuous sensor feedback (camera distance)
3. Arduino adjusts drive motors based on commands
4. When close: Pi sends `"screwDown"`
5. When screw reaches bottom limit: Pi sends `"closeClamp"`
6. Sorter receives `"sepColor"` command to initialize for specific bin
7. Each ball receives `"toColor"` command for routing
8. Process repeats until 12 balls sorted or timeout occurs

## Safety Features

- **Limit Switches:** Prevent mechanical over-extension
- **Deadzone:** Ignore small trigger inputs (<0.2) to prevent drift
- **PWM Clamping:** Motor values stay within 1000-2000µs range
- **State Flags:** Track mechanical state to prevent conflicting commands
- **Initialization:** Recalibrate color sorter on startup and reset

## Calibration

### Color Sorter Calibration (Automatic)
Runs on every `setup()`:
1. Move to limit switch 2 (center-right position)
2. Move to limit switch 4 (center-left position)
3. Ready for operation

### Motor Calibration
- Motors use standard ESC calibration (1500µs center, 1000-2000µs range)
- Servo calibration may need adjustment for gate positions
- Adjust `openGatePosition` and `closeGatePosition` constants if needed

## Dependencies

- `Servo.h` - Motor and servo control library (standard Arduino library)

## Troubleshooting

**Motors not responding:**
- Check baud rate (9600)
- Verify serial command format (ID:Value or String)
- Ensure commands end with `\n`

**Servos jittering:**
- Check power supply voltage
- Verify pin assignments match hardware
- Adjust servo PWM refresh rates if needed

**Limit switches not working:**
- Test with Serial.println(digitalRead(pin))
- Verify INPUT_PULLUP configuration
- Check physical switch wiring

**Color sorter stuck:**
- Manually run calibration sequence
- Check for mechanical obstructions
- Verify limit switches are properly positioned

**Unresponsive after command:**
- May be waiting for limit switch in color routing function
- Send neutral or reset command
- Check Arduino serial monitor for "Err" messages
