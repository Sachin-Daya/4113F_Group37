import pandas as pd
import random
from datetime import datetime, timedelta

# Define penguins and date range
penguin_ids = [f'P-{i:03d}' for i in range(1, 16)]  # P-001 to P-015
start_date = datetime(2023, 10, 1)
num_days = 30

data = []

for penguin_id in penguin_ids:
    # Starting attributes (within a healthy range)
    start_weight = random.randint(4700, 5200)
    start_width = round(random.uniform(18.0, 20.0), 1)
    height = round(random.uniform(60.0, 65.0), 1)  # Height stays nearly constant

    # Generate 5–8 readings per penguin
    num_entries = random.randint(5, 8)
    day_offsets = sorted(random.sample(range(num_days), num_entries))

    for i, offset in enumerate(day_offsets):
        date = start_date + timedelta(days=offset)

        # Apply decreasing trend over time with small noise
        weight = start_weight - i * random.randint(60, 120) + random.randint(-30, 30)
        weight = max(weight, 3200)  # Ensure weight doesn't drop unrealistically

        width = start_width - i * random.uniform(0.1, 0.2) + random.uniform(-0.1, 0.1)
        width = round(max(width, 15.0), 1)

        # Small fluctuation in height
        h_variation = random.uniform(-0.3, 0.3)
        measured_height = round(height + h_variation, 1)

        data.append({
            "Date": date.strftime('%Y-%m-%d'),
            "Penguin ID": penguin_id,
            "Weight (g)": weight,
            "Height (cm)": measured_height,
            "Width (cm)": width
        })

# Convert to DataFrame and save
df = pd.DataFrame(data)
df = df.sort_values(["Penguin ID", "Date"])
df.to_csv("penguin_data.csv", index=False)

# Preview top 10 rows
df.head(10)
