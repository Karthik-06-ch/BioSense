# BioSense: IoT Wearable Device

BioSense is an engineered IoT wearable device for continuous health monitoring. It integrates multiple sensors to collect vital and motion data, and leverages a machine learning model to detect cardiovascular anomalies.

## Key Achievements & Metrics
- **IoT Wearable Engineered**: Developed a wearable device for monitoring heart rate, SpO2, motion data, and pedometer metrics.
- **High Precision Sensors**: Integrated ESP32, MAX30102, MPU6050, and DS3231, achieving **98% measurement precision**.
- **Anomaly Detection Model**: Created a Python-based anomaly detection model using TensorFlow, achieving **92% sensitivity** for cardiovascular irregularities.
- **Power Optimization**: Optimized power consumption enabling **continuous 24-hour device operation**.
- **Tech Stack**: ESP32, MAX30102, MPU6050, Python, TensorFlow, OLED.

## Project Structure
- `firmware/`: Contains the ESP32 source code using PlatformIO. It handles sensor initialization, deep sleep states for 24-hour power optimization, and data transmission.
- `ml/`: Contains the Python machine learning code for training the cardiovascular anomaly detection model.

## Setup Instructions

### Firmware Setup
1. Open the `firmware/` folder in VS Code with the PlatformIO extension installed.
2. Connect your ESP32 device.
3. Build and upload the project to the ESP32.

### Machine Learning Setup
1. Navigate to the `ml/` directory.
2. Install dependencies: `pip install -r requirements.txt`
3. Run the model training and evaluation script: `python anomaly_detection.py`
