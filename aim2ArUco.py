import cv2
import numpy as np
import pyautogui
import keyboard
import mss
import time
import math
import win32api, win32con

# --- Configuration ---
SCREEN_WIDTH, SCREEN_HEIGHT = pyautogui.size()
MONITOR = {"top": 0, "left": 0, "width": SCREEN_WIDTH, "height": SCREEN_HEIGHT}

ARUCO_DICT = (
    cv2.aruco.DICT_4X4_50
)  # Using a 4x4 dictionary with 50 IDs, adjust if needed
AIM_SPEED = 0.1  # Adjust this value to change aim speed (lower is faster)
OFFSET_Y = (
    0  # Adjust this to fine-tune vertical aim, e.g., to aim at the center of the head
)


def capture_screenshot(sct):
    """Captures a screenshot of the primary monitor."""
    sct_img = sct.grab(MONITOR)
    return np.array(sct_img)


def detect_aruco_markers(frame):
    """Detects ArUco markers in the given frame and returns their centers."""
    if frame is None:
        return []
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    aruco_dict = cv2.aruco.getPredefinedDictionary(ARUCO_DICT)
    aruco_params = cv2.aruco.DetectorParameters()
    detector = cv2.aruco.ArucoDetector(aruco_dict, aruco_params)
    corners, ids, rejected = detector.detectMarkers(gray)

    marker_centers = []
    if ids is not None:
        for i, corner in enumerate(corners):
            # Calculate the center of the marker
            center_x = int(corner[0][:, 0].mean())
            center_y = int(corner[0][:, 1].mean())
            marker_centers.append((center_x, center_y))
    return marker_centers


def find_closest_marker(marker_centers, screen_center_x, screen_center_y):
    """Finds the closest marker to the center of the screen."""
    if not marker_centers:
        return None

    closest_marker = None
    min_distance = float("inf")

    for mx, my in marker_centers:
        distance = math.hypot(mx - screen_center_x, my - screen_center_y)
        if distance < min_distance:
            min_distance = distance
            closest_marker = (mx, my)
    return closest_marker


def aim_at_target(target_x, target_y, screen_center_x, screen_center_y):
    """Moves the mouse relative to the current position to aim at the target."""
    # Calculate the distance from the center of the screen to the target
    dx = target_x - screen_center_x
    dy = target_y - screen_center_y + OFFSET_Y

    # Use relative movement (MOUSEEVENTF_MOVE) instead of absolute
    win32api.mouse_event(win32con.MOUSEEVENTF_MOVE, dx, dy, 0, 0)


def main():
    print("Starting aim2ArUco script. Press and hold LEFT SHIFT to activate auto-aim.")
    print("Press CTRL+C in the console to stop the script.")

    screen_center_x = SCREEN_WIDTH // 2
    screen_center_y = SCREEN_HEIGHT // 2

    with mss.mss() as sct:
        while True:
            if keyboard.is_pressed("shift"):
                screenshot = capture_screenshot(sct)
                if screenshot is None:
                    continue
                marker_centers = detect_aruco_markers(screenshot)

                if marker_centers:
                    closest_marker = find_closest_marker(
                        marker_centers, screen_center_x, screen_center_y
                    )
                    if closest_marker:
                        aim_at_target(
                            closest_marker[0],
                            closest_marker[1],
                            screen_center_x,
                            screen_center_y,
                        )
            time.sleep(0.01)  # Small delay to reduce CPU usage and make it less twitchy


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Script stopped by user.")
    except Exception as e:
        print(f"An error occurred: {e}")
