import streamlit as st
import pandas as pd
import os
import matplotlib.pyplot as plt
import seaborn as sns
import base64 

# CSV path defined here instead of front end
CSV_PATH = os.path.join(os.getcwd(), r"C:\Users\Naseeka\Downloads\Object-Detection-Size-Measurement-master\penguin_data.csv")
IMAGE_FOLDER = os.path.join(os.getcwd(), "images")
BACKGROUND_IMAGES = {
    "Home": "bg_home.jpg",
    "📊 Raw Data + Images": "bg_raw.jpg",
    "📈 Summary": "bg_summary.jpg",
    "🔍 Search by RFID": "bg_lookup.jpg",
    "🚨 At-Risk Penguins": "bg_risk.jpg",
    "🚪 Logout": "backgrounds/bg_home.jpg"
}



def set_background(image_file):
    with open(image_file, "rb") as f:
        encoded_string = base64.b64encode(f.read()).decode()
    st.markdown(
        f"""
        <style>
        .stApp {{
            background-image: url("data:image/jpg;base64,{encoded_string}");
            background-size: cover;
            background-repeat: no-repeat;
            background-attachment: fixed;
        }}
        </style>
        """,
        unsafe_allow_html=True
    )

def load_data():
    df = pd.read_csv(CSV_PATH)
    df.columns = df.columns.str.strip()
    return df

def display_summary(df):
    st.subheader("📊 Summary Statistics")
    summary = df.groupby("Penguin ID").agg({
        "Weight (g)": ["mean", "min", "max"],
        "Height (cm)": ["mean"],
        "Width (cm)": ["mean"]
    })
    st.dataframe(summary)
    summary_csv = summary.to_csv().encode("utf-8")
    st.download_button("⬇️ Download Summary (CSV)", summary_csv, "penguin_summary.csv", "text/csv")
    
    # Plot
    fig, ax = plt.subplots()
    sns.barplot(data=df, x="Penguin ID", y="Weight (g)", ax=ax)
    st.pyplot(fig)

def app():
    st.set_page_config("Group37 Saves the Penguins", layout="wide")
    st.title("🐧 Group37 Saves the Penguins")

    df = load_data()

    menu = ["Home", "📊 Raw Data + Images", "📈 Summary", "🔍 Search by RFID", "🚨 At-Risk Penguins", "🚪 Logout"]
    page = st.sidebar.selectbox("Choose a view:", menu)

    set_background(BACKGROUND_IMAGES.get(page, ""))

    if page == "Home":
        st.header("Welcome to the Penguin Monitoring System")
        st.markdown("Use the sidebar to navigate between raw data, summaries, search, and more.")

    elif page == "📊 Raw Data + Images":
        st.subheader("📊 Raw Data")
        st.dataframe(df)
        csv_raw = df.to_csv(index=False).encode('utf-8')
        st.download_button("⬇️ Download Full Dataset (CSV)", csv_raw, "penguin_data.csv", "text/csv")

        st.subheader("📷 Penguin Images")
        for penguin_id in df["Penguin ID"].unique():
            st.markdown(f"**{penguin_id}**")
            img_path = os.path.join(IMAGE_FOLDER, f"{penguin_id}.jpg")
            if os.path.exists(img_path):
                st.image(img_path, width=250)
            else:
                st.info(f"No image found for {penguin_id}")

    elif page == "📈 Summary":
        display_summary(df)

    elif page == "🔍 Search by RFID":
        st.subheader("🔍 Search for a Penguin by RFID")
        rfid = st.text_input("Enter Penguin ID (e.g., P-001):")
        if rfid:
            result = df[df["Penguin ID"] == rfid.upper()]
            if not result.empty:
                st.dataframe(result)
                csv_result = result.to_csv(index=False).encode("utf-8")
                st.download_button("⬇️ Download Penguin Data", csv_result, f"{rfid}_data.csv", "text/csv")

                # Convert and sort by date
                result["Date"] = pd.to_datetime(result["Date"])
                result = result.sort_values("Date")

                # Calculate BMI
                result["BMI"] = result["Weight (g)"] / (result["Height (cm)"] ** 2)

                # Shared x-axis range buffer
                dates = result["Date"]
                date_min = dates.min() - pd.Timedelta(days=2)
                date_max = dates.max() + pd.Timedelta(days=2)

                # Plot Weight Over Time
                st.subheader("📉 Weight Over Time")
                fig1, ax1 = plt.subplots()
                ax1.plot(dates, result["Weight (g)"], marker="o", linestyle='-')
                ax1.set_ylabel("Weight (g)")
                ax1.set_xlabel("Date")
                ax1.set_title("Weight Over Time")
                ax1.set_xlim([date_min, date_max])
                plt.setp(ax1.get_xticklabels(), rotation=90)
                st.pyplot(fig1)

                # Plot Width Over Time
                st.subheader("📏 Width Over Time")
                fig2, ax2 = plt.subplots()
                ax2.plot(dates, result["Width (cm)"], marker="o", linestyle='-', color='green')
                ax2.set_ylabel("Width (cm)")
                ax2.set_xlabel("Date")
                ax2.set_title("Width Over Time")
                ax2.set_xlim([date_min, date_max])
                plt.setp(ax2.get_xticklabels(), rotation=90)
                st.pyplot(fig2)

                # Plot BMI Over Time
                st.subheader("📐 BMI Over Time")
                fig3, ax3 = plt.subplots()
                ax3.plot(dates, result["BMI"], marker="o", linestyle='-', color='purple')
                ax3.set_ylabel("BMI")
                ax3.set_xlabel("Date")
                ax3.set_title("BMI Over Time")
                ax3.set_xlim([date_min, date_max])
                plt.setp(ax3.get_xticklabels(), rotation=90)
                st.pyplot(fig3)

            else:
                st.error("No penguin found with that ID.")

    elif page == "🚨 At-Risk Penguins":
        st.subheader("🚨 Penguins Underweight (< 4000g)")
        risky = df[df["Weight (g)"] < 4000]
        if not risky.empty:
            st.warning("These penguins may need attention:")
            st.dataframe(risky)
            risky_csv = risky.to_csv(index=False).encode("utf-8")
            st.download_button("⬇️ Download At-Risk Penguins (CSV)", risky_csv, "at_risk_penguins.csv", "text/csv")
        else:
            st.success("All penguins appear healthy!")

    elif page == "🚪 Logout":
        st.header("👋 Goodbye!")
        st.success("You have exited the Penguin Monitoring System. Close the browser tab or terminal to fully exit.")

if __name__ == '__main__':
    app()
