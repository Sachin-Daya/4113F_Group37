import streamlit as st
import pandas as pd
import os

# Title
st.title("🐧 Penguin Monitoring Dashboard")

# Upload CSV file
uploaded_file = st.file_uploader("Upload Penguin Data CSV", type=["csv"])

if uploaded_file:
    # Load CSV
    df = pd.read_csv(uploaded_file, sep=';')  # Use sep=';' as in your dataset
    df.columns = [col.strip() for col in df.columns]  # Clean column names

    # User Options
    option = st.selectbox(
        "Choose an action:",
        [
            "1. View raw data with images",
            "2. View data summary by penguins",
            "3. Search by Penguin ID (RFID)",
            "4. Identify at-risk penguins"
        ]
    )

    # Feature 1: Raw data with images
    if option.startswith("1"):
        st.subheader("📊 Raw Data")
        st.dataframe(df)

        st.subheader("📷 Penguin Images (if available)")
        image_folder = "images"  # Change this to your actual images folder
        for penguin_id in df["Penguin ID"].unique():
            img_path = os.path.join(image_folder, f"{penguin_id}.jpg")
            st.markdown(f"**{penguin_id}**")
            if os.path.exists(img_path):
                st.image(img_path, width=250)
            else:
                st.warning(f"No image found for {penguin_id}")

    # Feature 2: Summary
    elif option.startswith("2"):
        st.subheader("📈 Summary Statistics")
        st.write(df.groupby("Penguin ID").agg({
            "Weight (g)": ["mean", "min", "max"],
            "Height (cm)": ["mean"],
            
            "Width (cm)": ["mean"],
        }).round(2))

    # Feature 3: Search by Penguin ID
    elif option.startswith("3"):
        penguin_id = st.text_input("Enter Penguin ID (e.g., P-001):")
        if penguin_id:
            result = df[df["Penguin ID"] == penguin_id]
            if not result.empty:
                st.write(f"📋 Data for {penguin_id}")
                st.dataframe(result)
                img_path = os.path.join("images", f"{penguin_id}.jpg")
                if os.path.exists(img_path):
                    st.image(img_path, caption=penguin_id, width=300)
            else:
                st.error("Penguin ID not found.")

    # Feature 4: Identify at-risk penguins
    elif option.startswith("4"):
        st.subheader("🚨 At-Risk Penguins")
        risk_criteria = df["Weight (g)"] < 4000
        risky_penguins = df[risk_criteria]
        if not risky_penguins.empty:
            st.warning("These penguins are below healthy weight thresholds:")
            st.dataframe(risky_penguins)
        else:
            st.success("No penguins appear to be at risk based on weight.")

else:
    st.info("Upload a CSV file to begin.")
