# Complete AOSP Smart Home Integration Guide

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [ESP32 Setup](#esp32-setup)
3. [AOSP Source Setup](#aosp-source-setup)
4. [Application Integration](#application-integration)
5. [Building & Flashing](#building--flashing)
6. [Testing](#testing)
7. [Troubleshooting](#troubleshooting)

---

## 1. Prerequisites

### Hardware Requirements
- **ESP32 Development Board** (with relays on GPIO 10, 11, 12)
- **Android Device** (running AOSP build)
- **WiFi Network** (both devices on same network recommended)
- **Computer** (8GB+ RAM, 100GB+ storage for AOSP build)

### Software Requirements
```bash
# Ubuntu 20.04/22.04 recommended
sudo apt-get install git-core gnupg flex bison build-essential \
    zip curl zlib1g-dev gcc-multilib g++-multilib libc6-dev-i386 \
    libncurses5 lib32ncurses5-dev x11proto-core-dev libx11-dev \
    lib32z1-dev libgl1-mesa-dev libxml2-utils xsltproc unzip \
    fontconfig python3 openjdk-11-jdk
```

---

## 2. ESP32 Setup

### A. Hardware Connections
```
ESP32 Pin     →  Component
──────────────────────────────
GPIO 10       →  Relay 1 (Kitchen Light)
GPIO 11       →  Relay 2 (Bedroom Light)
GPIO 12       →  Relay 3 (Living Hall Light)
GPIO 16       →  Buzzer
GPIO 38       →  WS2812B LED (optional)
GPIO 4        →  Button (with pull-up)
3.3V          →  VCC (Sensors/LED)
GND           →  Common Ground
```

### B. Flash ESP32 Code
```bash
# Install PlatformIO
pip install platformio

# Create project
pio project init --board esp32dev

# Copy the ESP32 code provided earlier
# platformio.ini:
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    WiFi
    WebServer
    PubSubClient
    Adafruit_NeoPixel
monitor_speed = 115200

# Flash
pio run --target upload
pio device monitor
```

### C. Configure WiFi & MQTT
Edit in ESP32 code:
```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
```

### D. Test ESP32
```bash
# After flashing, monitor serial output
# You should see:
# - WiFi connection (IP address)
# - Web server running
# - MQTT connection status

# Test web interface
# Open browser: http://[ESP32_IP]

# Test MQTT
mosquitto_pub -h broker.hivemq.com -t smarthome/control -m "kitchen_light_on"
```

---

## 3. AOSP Source Setup

### A. Download AOSP Source
```bash
# Create workspace
mkdir ~/aosp
cd ~/aosp

# Initialize repo
repo init -u https://android.googlesource.com/platform/manifest -b android-13.0.0_r1

# Sync (takes hours, ~150GB)
repo sync -c -j8
```

### B. Add MQTT Libraries
```bash
cd external/
mkdir mqtt-android
cd mqtt-android

# Download Paho MQTT Android Service
wget https://repo.eclipse.org/content/repositories/paho-releases/org/eclipse/paho/org.eclipse.paho.android.service/1.1.1/org.eclipse.paho.android.service-1.1.1.aar

# Download Paho MQTT Java Client
wget https://repo.eclipse.org/content/repositories/paho-releases/org/eclipse/paho/org.eclipse.paho.client.mqttv3/1.2.5/org.eclipse.paho.client.mqttv3-1.2.5.jar
```

Create `Android.bp`:
```javascript
android_library_import {
    name: "org.eclipse.paho.android.service",
    aars: ["org.eclipse.paho.android.service-1.1.1.aar"],
    sdk_version: "current",
}

java_import {
    name: "org.eclipse.paho.client.mqttv3",
    jars: ["org.eclipse.paho.client.mqttv3-1.2.5.jar"],
    sdk_version: "current",
}
```

---

## 4. Application Integration

### A. Create App Directory
```bash
cd ~/aosp/packages/apps
mkdir SmartHomeControl
cd SmartHomeControl
```

### B. Create Directory Structure
```
SmartHomeControl/
├── Android.bp
├── AndroidManifest.xml
├── res/
│   ├── layout/
│   │   ├── activity_main.xml
│   │   └── device_item.xml
│   ├── values/
│   │   ├── strings.xml
│   │   └── colors.xml
│   ├── drawable/
│   │   ├── card_background.xml
│   │   └── icon_background.xml
│   └── xml/
│       └── widget_info.xml
└── src/com/android/smarthome/
    ├── MainActivity.java
    ├── MqttService.java
    ├── Device.java
    ├── DeviceType.java
    ├── DeviceAdapter.java
    ├── QuickSettingsTile.java
    └── BootReceiver.java
```

### C. Copy All Source Files
Copy all the Java and XML files provided earlier into appropriate directories.

### D. Create Android.bp
```javascript
android_app {
    name: "SmartHomeControl",
    srcs: ["src/**/*.java"],
    resource_dirs: ["res"],
    platform_apis: true,
    certificate: "platform",
    privileged: true,
    
    static_libs: [
        "androidx.appcompat_appcompat",
        "androidx.recyclerview_recyclerview",
        "androidx.cardview_cardview",
        "org.eclipse.paho.android.service",
        "org.eclipse.paho.client.mqttv3",
    ],
    
    optimize: {
        enabled: false,
    },
}
```

### E. Add to Device Makefile
```bash
# Edit your device's makefile
# Example: device/generic/goldfish/device.mk
nano device/generic/goldfish/device.mk

# Add at the end:
PRODUCT_PACKAGES += \
    SmartHomeControl
```

---

## 5. Building & Flashing

### A. Build AOSP
```bash
cd ~/aosp

# Setup environment
source build/envsetup.sh

# Choose target
lunch aosp_x86_64-eng  # For emulator
# OR
lunch aosp_arm64-userdebug  # For physical device

# Build (takes hours)
m -j$(nproc)
```

### B. Build Only the App (faster for testing)
```bash
cd ~/aosp
source build/envsetup.sh
lunch [your_target]

# Build just the app
m SmartHomeControl

# The APK will be at:
# out/target/product/[device]/system/app/SmartHomeControl/SmartHomeControl.apk
```

### C. Install on Device
```bash
# Method 1: Push to running AOSP device
adb root
adb remount
adb push out/target/product/[device]/system/app/SmartHomeControl/SmartHomeControl.apk /system/app/SmartHomeControl/
adb reboot

# Method 2: Flash full system image
cd ~/aosp
fastboot flashall -w
```

### D. Quick Development Build
For faster iteration during development:
```bash
# Build app only
mm SmartHomeControl

# Install
adb install -r out/target/product/[device]/system/app/SmartHomeControl/SmartHomeControl.apk

# View logs
adb logcat | grep SmartHome
```

---

## 6. Testing

### A. Verify Installation
```bash
# Check if app is installed
adb shell pm list packages | grep smarthome

# Should output:
# com.android.smarthome

# Check if service is running
adb shell dumpsys activity services | grep MqttService
```

### B. Test MQTT Connection
```bash
# From computer, publish test message
mosquitto_pub -h broker.hivemq.com -t smarthome/control -m "kitchen_light_on"

# Check Android logs
adb logcat | grep MqttService

# Should see:
# Message received - Topic: smarthome/control, Message: kitchen_light_on
```

### C. Test Full Workflow
1. **Open App** on Android device
2. **Check Connection Status** (should show "Connected")
3. **Toggle a device** (Kitchen Light)
4. **Verify ESP32** serial monitor shows command received
5. **Check ESP32 relay** activates (LED on relay board)
6. **Verify status update** comes back to Android app

### D. Test Quick Settings Tile
1. Swipe down notification shade
2. Edit quick settings
3. Add "Smart Home" tile
4. Tap tile to toggle all lights

---

## 7. Troubleshooting

### Common Issues

#### Issue 1: MQTT Connection Failed
```bash
# Check logs
adb logcat | grep MQTT

# Common causes:
# - No internet connection
# - Firewall blocking port 1883
# - Broker down (try: test.mosquitto.org)

# Test connectivity
adb shell ping broker.hivemq.com
adb shell nc -zv broker.hivemq.com 1883
```

#### Issue 2: App Crashes on Launch
```bash
# Check crash logs
adb logcat | grep AndroidRuntime

# Common causes:
# - Missing MQTT libraries
# - Permission denied
# - Missing resources

# Verify permissions
adb shell dumpsys package com.android.smarthome | grep permission
```

#### Issue 3: ESP32 Not Responding
```bash
# Check ESP32 serial output
pio device monitor

# Verify:
# - WiFi connected
# - MQTT connected
# - Correct topic subscription

# Test direct MQTT publish
mosquitto_pub -h broker.hivemq.com -t smarthome/control -m "status"
```

#### Issue 4: Build Errors
```bash
# Clean build
m clean
m SmartHomeControl

# Check dependency errors
mm SmartHomeControl 2>&1 | grep error

# Verify MQTT libs are present
ls external/mqtt-android/
```

#### Issue 5: Devices Not Updating
```bash
# Enable verbose logging in MainActivity.java
Log.d(TAG, "Message received: " + message);

# Monitor both sides
# Terminal 1: ESP32
pio device monitor

# Terminal 2: Android
adb logcat | grep SmartHome

# Verify message flow:
# Android -> MQTT Broker -> ESP32 -> Response -> Android
```

---

## Advanced Configuration

### A. Change MQTT Broker
```java
// In MqttService.java
private static final String BROKER_URL = "tcp://YOUR_BROKER:1883";

// If using authentication
options.setUserName("your_username");
options.setPassword("your_password".toCharArray());
```

### B. Add More Devices
```java
// In MainActivity.java - initializeDevices()
devices.add(new Device(
    "new_device_id",
    "New Device Name",
    "Type",
    DeviceType.LIGHT,
    13,  // GPIO pin
    false
));
```

### C. Enable SSL/TLS
```java
// In MqttService.java
private static final String BROKER_URL = "ssl://broker.hivemq.com:8883";

options.setSocketFactory(SSLSocketFactory.getDefault());
```

### D. Background Service Optimization
```java
// In MqttService.java onCreate()
options.setAutomaticReconnect(true);
options.setKeepAliveInterval(60);
options.setConnectionTimeout(10);

// Prevent service from being killed
@Override
public int onStartCommand(Intent intent, int flags, int startId) {
    return START_STICKY;
}
```

---

## Performance Tips

1. **Reduce MQTT Reconnect Attempts**
   - Set reasonable retry intervals (5-10 seconds)
   
2. **Optimize Network Usage**
   - Use QoS 0 for non-critical messages
   - Enable retained messages for status updates

3. **Battery Optimization**
   - Use AlarmManager for periodic checks
   - Implement doze mode whitelist

4. **Memory Management**
   - Limit log buffer size
   - Clear old messages periodically

---

## Security Recommendations

1. **Use Encrypted MQTT** (TLS/SSL)
2. **Implement Authentication** (username/password)
3. **Use App Signing** (release keys)
4. **Add Network Security Config**
5. **Implement Rate Limiting** on ESP32
6. **Use OTA Updates** for ESP32 firmware

---

## Next Steps

1. **Add Voice Control** via Google Assistant integration
2. **Implement Scheduling** for automated control
3. **Add Scenes** (preset configurations)
4. **Create Dashboard Widget** with real-time updates
5. **Implement User Management** (multiple users/roles)
6. **Add Energy Monitoring** (track power consumption)
7. **Create Automation Rules** (if-then-else logic)

---

## Resources

- **AOSP Documentation**: https://source.android.com/docs
- **Paho MQTT**: https://www.eclipse.org/paho/
- **ESP32 Arduino**: https://github.com/espressif/arduino-esp32
- **HiveMQ Broker**: https://www.hivemq.com/
- **MQTT Protocol**: https://mqtt.org/

---

## Support

For issues and questions:
1. Check ESP32 serial output
2. Check Android logcat
3. Verify network connectivity
4. Test MQTT broker separately
5. Check SELinux denials: `adb shell dmesg | grep denied`
