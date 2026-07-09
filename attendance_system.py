import cv2
import face_recognition
import os
import numpy as np
from datetime import datetime
import time
import requests

# ====== SETTINGS ======
IMAGE_DIR = 'student_images'
ATTENDANCE_FILE = 'Attendance.csv'
REQUIRED_SECONDS = 15  # Minimum duration in class (seconds)
cooldown_time = 5      # seconds before next detection allowed
wait_after_exit = 4    # seconds to wait after marking exit

# ====== ESP32 CAMERA SETTINGS ======
ESP32_URL = "http://YOUR_ESP32_IP:81/stream"

# ====== BLYNK SETTINGS ======
BLYNK_AUTH_TOKEN = "YOUR_BLYNK_AUTH_TOKEN"
VIRTUAL_PIN = "V5"

def send_to_blynk(value):
    """Send Occupied/Empty status to Blynk."""
    url = f"https://blynk.cloud/external/api/update?token={BLYNK_AUTH_TOKEN}&{VIRTUAL_PIN}={value}"
    try:
        res = requests.get(url, timeout=5)
        if res.status_code == 200:
            print(f"[BLYNK] Sent: {value}")
        else:
            print(f"[BLYNK ERROR] {res.status_code}: {res.text}")
    except requests.RequestException as e:
        print(f"[BLYNK ERROR] {e}")

# ====== DATA STRUCTURES ======
entry_log = {}         # {name: entry_time}
exit_ready = set()     # students allowed to exit (seen once)
waiting_for_exit = {}  # {name: last_seen_time}
last_status = None     # Track last sent status to avoid repeats
cooldown_until = 0     # Timestamp until which recognition is paused

# ====== ADD STUDENT ======
def create_student_folder(name):
    folder_path = os.path.join(IMAGE_DIR, name)
    os.makedirs(folder_path, exist_ok=True)
    return folder_path

def capture_student_images(name):
    print(f"[INFO] Capturing images for {name} from ESP32-CAM stream...")
    folder = create_student_folder(name)
    cap = cv2.VideoCapture(ESP32_URL)
    time.sleep(2)  # Wait for ESP32 stream to initialize
    count = 0

    while count < 2:
        ret, frame = cap.read()
        if not ret:
            print("[ERROR] Cannot access ESP32 stream.")
            break

        cv2.imshow(f"Capture - {name} (SPACE to save, Q to quit)", frame)
        key = cv2.waitKey(1) & 0xFF

        if key == 32:  # SPACE
            img_path = os.path.join(folder, f'{count + 1}.jpg')
            cv2.imwrite(img_path, frame)
            print(f"[SAVED] {img_path}")
            count += 1
        elif key == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

# ====== FACE ENCODING ======
def load_images_and_names():
    images = []
    names = []
    print("[INFO] Loading student images...")

    if not os.path.exists(IMAGE_DIR):
        return images, names

    for student in os.listdir(IMAGE_DIR):
        student_path = os.path.join(IMAGE_DIR, student)
        if os.path.isdir(student_path):
            for img_file in os.listdir(student_path):
                img_path = os.path.join(student_path, img_file)
                img = cv2.imread(img_path)
                if img is not None:
                    images.append(img)
                    names.append(student)
    return images, names

def generate_encodings(images, names):
    encodings = []
    filtered_names = []
    print("[INFO] Encoding faces...")

    for img, name in zip(images, names):
        rgb_img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        boxes = face_recognition.face_locations(rgb_img)
        enc = face_recognition.face_encodings(rgb_img, boxes)
        if enc:
            encodings.append(enc[0])
            filtered_names.append(name)
    return encodings, filtered_names

# ====== ATTENDANCE LOGIC ======
def mark_entry(name):
    entry_log[name] = time.time()
    print(f"[ENTRY] {name} at {datetime.now().strftime('%H:%M:%S')}")

def mark_exit(name):
    exit_time = time.time()
    entry_time = entry_log.get(name, exit_time)
    duration = exit_time - entry_time

    if duration >= REQUIRED_SECONDS:
        entry_str = datetime.fromtimestamp(entry_time).strftime('%H:%M:%S')
        exit_str = datetime.fromtimestamp(exit_time).strftime('%H:%M:%S')
        with open(ATTENDANCE_FILE, 'a') as f:
            f.write(f"{name},{entry_str},{exit_str},{int(duration)}s,Present\n")
        print(f"[RECORDED] {name} | Duration: {int(duration)}s")
    else:
        print(f"[SKIPPED] {name} stayed only {int(duration)}s")

    # Reset state
    entry_log.pop(name, None)
    exit_ready.discard(name)
    waiting_for_exit.pop(name, None)

# ====== ATTENDANCE SYSTEM ======
def start_attendance():
    global last_status, cooldown_until
    images, names = load_images_and_names()
    if not images:
        print("[ERROR] No student images found.")
        return

    encodings, final_names = generate_encodings(images, names)
    print("[INFO] Connecting to ESP32-CAM stream for attendance...")

    cap = cv2.VideoCapture(ESP32_URL)
    time.sleep(2)  # wait for stream to stabilize

    while True:
        ret, frame = cap.read()
        if not ret:
            print("[ERROR] Failed to grab frame from ESP32.")
            break

        current_time = time.time()

        # If in cooldown, just display frame, skip recognition and attendance logic
        if current_time < cooldown_until:
            # Show occupancy status with cooldown indicator
            if len(entry_log) == 0:
                status = "OFF"
                color = (0, 0, 255)  # Red
            else:
                status = "ON"
                color = (0, 255, 0)  # Green

            if status != last_status:
                send_to_blynk(status)
                last_status = status

            cv2.putText(frame, f"Classroom is {status.upper()} (Cooldown)", (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.0, color, 2)

            cv2.imshow("Smart Attendance System", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            continue  # Skip rest of loop

        # Resize and convert color for faster face recognition
        small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
        rgb_small = cv2.cvtColor(small_frame, cv2.COLOR_BGR2RGB)

        face_locations = face_recognition.face_locations(rgb_small)
        face_encodings = face_recognition.face_encodings(rgb_small, face_locations)

        for face_encoding, face_location in zip(face_encodings, face_locations):
            matches = face_recognition.compare_faces(encodings, face_encoding)
            distances = face_recognition.face_distance(encodings, face_encoding)

            if matches:
                best_match = np.argmin(distances)
                if matches[best_match]:
                    name = final_names[best_match]

                    # Draw bounding box on original frame
                    top, right, bottom, left = [v * 4 for v in face_location]
                    cv2.rectangle(frame, (left, top), (right, bottom), (0, 255, 0), 2)
                    cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 255, 0), cv2.FILLED)
                    cv2.putText(frame, name, (left + 6, bottom - 6),
                                cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

                    # Attendance logic
                    if name not in entry_log and name not in exit_ready:
                        mark_entry(name)

                    elif name in entry_log and name not in exit_ready:
                        print(f"[READY TO EXIT] {name} - Please leave and come back to mark exit.")
                        exit_ready.add(name)
                        waiting_for_exit[name] = current_time

                    elif name in exit_ready:
                        last_seen = waiting_for_exit.get(name, 0)
                        if current_time - last_seen > cooldown_time:
                            mark_exit(name)
                            print("[INFO] Entering cooldown...")
                            cooldown_until = current_time + wait_after_exit

        # ====== CLASSROOM OCCUPANCY STATUS ======
        if len(entry_log) == 0:
            status = "OFF"
            color = (0, 0, 255)  # Red
        else:
            status = "ON"
            color = (0, 255, 0)  # Green

        if status != last_status:
            send_to_blynk(status)
            last_status = status

        cv2.putText(frame, f"Classroom is {status.upper()}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1.0, color, 2)

        cv2.imshow("Smart Attendance System", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

# ====== MAIN MENU ======
if __name__ == "__main__":
    print("""
========= FACE RECOGNITION ATTENDANCE =========
[1] Add New Student
[2] Start Attendance System
===============================================
    """)

    choice = input("Enter choice (1 or 2): ").strip()

    if choice == '1':
        student_name = input("Enter student name: ").strip()
        if student_name:
            capture_student_images(student_name)
        else:
            print("[ERROR] Invalid name.")
    elif choice == '2':
        os.makedirs(IMAGE_DIR, exist_ok=True)
        if not os.path.exists(ATTENDANCE_FILE):
            with open(ATTENDANCE_FILE, 'w') as f:
                f.write("Name,Entry,Exit,Duration,Status\n")
        start_attendance()
    else:
        print("[ERROR] Invalid option.")
