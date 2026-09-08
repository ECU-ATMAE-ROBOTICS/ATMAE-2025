# ATMAE 2025 Robot - TODO List

## 1. Limit Switch Solution
Investigate a better solution to how the limit switches stop the screw and prevent it from going past its allowed position. Currently, a small delay is used, but this causes us to lose control of the bot while the delay is active.

**Work Location:** Arduino file where the `parseData()` function calls the screw movement function.

---

## 2. Color Detection Accuracy
Color detect sometimes sees incorrect ball colors.

**Steps:**
- Use the test files found on the Pi's code base (not listed in this repo) to test color detect
- If incorrectness can be observed, use the `test_hsv.py` script to get the HSV values of the image
- Update the ranges found in `colordetect.py` based on the HSV values

---

## 3. Autonomous Mode Testing ⚠️ 
We have the code set up for autonomous mode, but we never got a solid shot at testing it. Review the documentation for the code found in `/docs` and see if you can have the robot reliably move to the bins automatically.

**Note** 
* If you wanna do this, make sure that you have a laptop/server with a GPU to run the YOLO model on
* If you want to work on this at the school, make sure both the pi and the GPU server are connected to the ATMAE Router. This won't work on school wifi.

