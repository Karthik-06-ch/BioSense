import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input, Dense, Dropout
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import classification_report, confusion_matrix, recall_score

# Set seed for reproducibility
np.random.seed(42)
tf.random.set_seed(42)

def generate_synthetic_data(num_samples=10000):
    # Simulate data coming from the ESP32 (Heart Rate, SpO2, Accelerometer XYZ)
    # Normal data
    normal_data = np.random.normal(loc=[70, 98, 0.5, 0.5, 9.8], scale=[10, 1, 0.1, 0.1, 0.1], size=(int(num_samples*0.9), 5))
    
    # Abnormal data (Cardiovascular irregularities: e.g. very high/low HR, dropped SpO2)
    abnormal_data = np.random.normal(loc=[120, 92, 1.5, 1.5, 12.0], scale=[20, 3, 0.5, 0.5, 2.0], size=(int(num_samples*0.1), 5))
    
    data = np.vstack([normal_data, abnormal_data])
    labels = np.hstack([np.zeros(len(normal_data)), np.ones(len(abnormal_data))])
    
    return data, labels

def build_autoencoder(input_dim):
    input_layer = Input(shape=(input_dim,))
    
    # Encoder
    encoded = Dense(32, activation='relu')(input_layer)
    encoded = Dropout(0.2)(encoded)
    encoded = Dense(16, activation='relu')(encoded)
    
    # Decoder
    decoded = Dense(32, activation='relu')(encoded)
    decoded = Dropout(0.2)(decoded)
    decoded = Dense(input_dim, activation='linear')(decoded)
    
    autoencoder = Model(inputs=input_layer, outputs=decoded)
    autoencoder.compile(optimizer='adam', loss='mse')
    
    return autoencoder

if __name__ == '__main__':
    print("Generating synthetic physiological data...")
    X, y = generate_synthetic_data(10000)
    
    # Split data (use only normal data for training the autoencoder)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, stratify=y, random_state=42)
    
    X_train_normal = X_train[y_train == 0]
    
    # Scale data
    scaler = StandardScaler()
    X_train_normal_scaled = scaler.fit_transform(X_train_normal)
    X_test_scaled = scaler.transform(X_test)
    
    print("Building Autoencoder Anomaly Detection Model...")
    autoencoder = build_autoencoder(X_train.shape[1])
    
    print("Training model...")
    autoencoder.fit(X_train_normal_scaled, X_train_normal_scaled, 
                    epochs=50, batch_size=64, validation_split=0.1, 
                    verbose=0)
    
    print("Evaluating model...")
    # Get reconstruction errors
    reconstructions = autoencoder.predict(X_test_scaled)
    mse = np.mean(np.power(X_test_scaled - reconstructions, 2), axis=1)
    
    # Determine threshold for anomalies based on training data
    train_reconstructions = autoencoder.predict(X_train_normal_scaled)
    train_mse = np.mean(np.power(X_train_normal_scaled - train_reconstructions, 2), axis=1)
    threshold = np.percentile(train_mse, 95) # 95th percentile
    
    y_pred = (mse > threshold).astype(int)
    
    sensitivity = recall_score(y_test, y_pred)
    print(f"\n--- Results ---")
    print(f"Target Sensitivity: > 92%")
    print(f"Achieved Sensitivity: {sensitivity * 100:.2f}%")
    
    if sensitivity >= 0.92:
        print("Success: Model meets the 92% sensitivity requirement for cardiovascular irregularities.")
    else:
        print("Model needs tuning to reach 92% sensitivity.")
        
    print("\nClassification Report:")
    print(classification_report(y_test, y_pred))
    
    # Save the model
    autoencoder.save("cardio_anomaly_detector.h5")
    print("Model saved to cardio_anomaly_detector.h5")
