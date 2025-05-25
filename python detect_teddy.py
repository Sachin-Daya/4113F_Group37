import os
import re
import csv
import logging
import cv2
import numpy as np
from datetime import datetime
from ultralytics import YOLO
import cv2.aruco as aruco

# --------------------------- Configuration ---------------------------
INPUT_DIR = r"C:\Users\Naseeka\Desktop\Academic Work\EEE4113\Elephants"
OUTPUT_DIR = r"C:\Users\Naseeka\Desktop\Academic Work\EEE4113\ElephantsResults"
ARUCO_SIZE_CM = 5
ARUCO_DICT = aruco.DICT_5X5_50
TEDDY_CLASS_ID = 77
MODEL_PATH = "yolov8m-seg.pt"
SAVE_ANNOTATED_IMAGES = True
MASTER_CSV = "all_teddies.csv"
# ---------------------------------------------------------------------

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def calculate_pixels_per_cm(corners):
    """Calculate pixels per centimeter using ArUco marker corners."""
    corners = corners.reshape((4, 2))
    sides = [np.linalg.norm(corners[i] - corners[(i + 1) % 4]) for i in range(4)]
    avg_side = np.mean(sides)
    return avg_side / ARUCO_SIZE_CM

def parse_filename(filename):
    """Parse filenames with dates in DDMMYYYY format (no dashes)."""
    name_part = os.path.splitext(filename)[0]
    parts = name_part.split('_')
    if len(parts) != 3:
        return None, None, None

    rfid = parts[0]
    weight_str = parts[1].lower().replace('kg', '')
    date_str = parts[2]

    if not re.match(r'^RFID\d+$', rfid):
        return None, None, None

    try:
        weight = float(weight_str)
    except ValueError:
        return None, None, None

    if len(date_str) == 8 and date_str.isdigit():
        try:
            date_obj = datetime.strptime(date_str, "%d%m%Y")
            formatted_date = date_obj.strftime("%Y-%m-%d")
            return rfid, weight, formatted_date
        except ValueError:
            pass

    return None, None, None

def process_image(image_path):
    """Process an image to detect teddy bear dimensions using ArUco and YOLO segmentation contours."""
    try:
        image = cv2.imread(image_path)
        if image is None:
            logging.error(f"Failed to load image: {image_path}")
            return None

        # ArUco Detection
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        aruco_dict = aruco.getPredefinedDictionary(ARUCO_DICT)
        parameters = aruco.DetectorParameters()
        parameters.cornerRefinementMethod = aruco.CORNER_REFINE_SUBPIX

        corners, ids, _ = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
        if ids is None:
            logging.error(f"No ArUco markers detected in {image_path}")
            return None

        pixels_per_cm = calculate_pixels_per_cm(corners[0])
        logging.info(f"Pixels per cm: {pixels_per_cm:.2f}")

        # YOLO Detection
        model = YOLO(MODEL_PATH)
        results = model(image)[0]

        teddies = []
        for i, box in enumerate(results.boxes):
            if int(box.cls) == TEDDY_CLASS_ID:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                height_px = y2 - y1
                width_px = x2 - x1
                height_cm = height_px / pixels_per_cm
                width_cm = width_px / pixels_per_cm
                teddies.append({
                    'height_cm': round(height_cm, 2),
                    'width_cm': round(width_cm, 2)
                })

                # --- Draw contour using segmentation mask ---
                if results.masks is not None:
                    mask = results.masks.data[i].cpu().numpy().astype(np.uint8) * 255
                    mask = cv2.resize(mask, (image.shape[1], image.shape[0]))
                    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
                    cv2.drawContours(image, contours, -1, (0, 255, 0), 2)

                # Label with height
                label = f"{height_cm:.1f}cm"
                cv2.putText(image, label, (x1, y1 - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        if SAVE_ANNOTATED_IMAGES:
            output_filename = f"{os.path.splitext(os.path.basename(image_path))[0]}_annotated.jpg"
            output_path = os.path.join(OUTPUT_DIR, output_filename)
            cv2.imwrite(output_path, image)
            logging.info(f"Saved annotated image to {output_path}")

        return teddies

    except Exception as e:
        logging.error(f"Error processing {image_path}: {str(e)}", exc_info=True)
        return None

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    for filename in os.listdir(INPUT_DIR):
        if not filename.lower().endswith(('.jpg', '.jpeg', '.png')):
            continue

        image_path = os.path.join(INPUT_DIR, filename)
        logging.info(f"Processing {image_path}")

        rfid, weight, date = parse_filename(filename)
        if not rfid:
            logging.error(f"Invalid filename format for {filename}. Skipping.")
            continue

        teddies = process_image(image_path)
        if not teddies:
            logging.warning(f"No teddies detected in {filename}. Skipping.")
            continue

        for teddy in teddies:
            rfid_csv = os.path.join(OUTPUT_DIR, f"{rfid}.csv")
            with open(rfid_csv, 'a', newline='') as f:
                writer = csv.writer(f)
                if os.stat(rfid_csv).st_size == 0:
                    writer.writerow(["Weight (kg)", "Date", "Height (cm)", "Width (cm)"])
                writer.writerow([weight, date, teddy['height_cm'], teddy['width_cm']])

            master_csv = os.path.join(OUTPUT_DIR, MASTER_CSV)
            with open(master_csv, 'a', newline='') as f:
                writer = csv.writer(f)
                if os.stat(master_csv).st_size == 0:
                    writer.writerow(["RFID", "Weight (kg)", "Date", "Height (cm)", "Width (cm)"])
                writer.writerow([rfid, weight, date, teddy['height_cm'], teddy['width_cm']])

        logging.info(f"Logged {len(teddies)} teddies from {filename}")

if __name__ == "__main__":
    main()
