import os
import csv
import shutil
import logging
from datetime import datetime, timedelta
from pathlib import Path

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('hourly_image_matching.log'),
        logging.StreamHandler()
    ]
)

def parse_image_time(filename):
    """Extracts datetime from image filename (format: YYYYMMDD-HHMMSS)"""
    try:
        return datetime.strptime(filename.split('.')[0], "%Y%m%d-%H%M%S")
    except ValueError:
        return None

def parse_record_time(date_str, time_str):
    """Combines date and time columns from spreadsheet"""
    try:
        return datetime.strptime(f"{date_str} {time_str}", "%Y/%m/%d %H:%M:%S")
    except ValueError:
        return None

def get_hour_key(penguin_id, dt):
    """Creates a unique key for each ID-hour combination"""
    return f"{penguin_id}_{dt.strftime('%Y%m%d_%H')}"

def find_matching_image(record_dt, image_files, tolerance=30):
    """
    Finds image with timestamp within tolerance window (seconds)
    Returns (best_match, time_difference) or (None, None)
    """
    best_match = None
    min_diff = tolerance + 1
    
    for img_file in image_files:
        img_dt = parse_image_time(img_file.name)
        if img_dt is None:
            continue
            
        time_diff = abs((record_dt - img_dt).total_seconds())
        if time_diff <= tolerance and time_diff < min_diff:
            min_diff = time_diff
            best_match = img_file
    
    return (best_match, min_diff) if best_match else (None, None)

def process_data_with_hourly_limit(data_file_path, image_dir, output_dir="Hourly_Images"):
    """Processes data with one image per ID per hour limit"""
    try:
        image_dir = Path(image_dir)
        output_dir = Path(output_dir)
        output_dir.mkdir(exist_ok=True)
        
        # Track processed ID-hour combinations
        processed_hours = set()
        matched_count = 0
        skipped_count = 0

        # Pre-scan all available images by date
        date_folders = {}
        for date_folder in image_dir.iterdir():
            if date_folder.is_dir():
                date_str = date_folder.name
                date_folders[date_str] = list(date_folder.glob("*.jp*")) + list(date_folder.glob("*.png"))

        with open(data_file_path, 'r', newline='') as csvfile:
            reader = csv.DictReader(csvfile)
            
            for row in reader:
                try:
                    penguin_id = row['ID'].strip()
                    record_dt = parse_record_time(row['Date'], row['Time'])
                    if record_dt is None:
                        continue
                    
                    hour_key = get_hour_key(penguin_id, record_dt)
                    if hour_key in processed_hours:
                        logging.debug(f"Skipping {penguin_id} at {record_dt} - already processed this hour")
                        skipped_count += 1
                        continue
                    
                    date_str = record_dt.strftime("%Y%m%d")
                    if date_str not in date_folders:
                        continue
                    
                    image_match, time_diff = find_matching_image(
                        record_dt,
                        date_folders[date_str]
                    )
                    
                    if image_match:
                        # Create new filename: ID_DateTime_Weight.ext
                        new_name = (f"{penguin_id}_"
                                   f"{record_dt.strftime('%Y%m%d_%H%M%S')}_"
                                   f"{row.get('Weight', 'NA')}{image_match.suffix}")
                        
                        shutil.copy2(
                            image_match,
                            output_dir / new_name
                        )
                        processed_hours.add(hour_key)
                        matched_count += 1
                        logging.info(f"Matched {penguin_id} at {record_dt} (Δ {time_diff:.1f}s)")
                    
                except Exception as e:
                    logging.error(f"Error processing row: {str(e)}")

        logging.info(f"\nProcessing complete:\n"
                    f"- Matched images: {matched_count}\n"
                    f"- Skipped duplicates: {skipped_count}\n"
                    f"- Unique ID-hour combinations: {len(processed_hours)}\n"
                    f"- Output directory: {output_dir.resolve()}")

    except Exception as e:
        logging.error(f"Fatal error: {str(e)}", exc_info=True)

if __name__ == "__main__":
    data_path = r"C:\Users\Naseeka\Downloads\Object-Detection-Size-Measurement-master\Input_Images\ESP32_RFID_PenguinData - PenguinData.csv"
    image_dir = r"C:\Users\Naseeka\Downloads\Object-Detection-Size-Measurement-master\Input_Images"
    
    process_data_with_hourly_limit(data_path, image_dir)