package com.android.smarthome;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import org.json.JSONObject;
import java.util.ArrayList;
import java.util.List;

/**
 * Main Activity for ESP32 Smart Home Control
 * 
 * Features:
 * - MQTT connection to ESP32
 * - Real-time device control
 * - Status monitoring
 * - Quick actions
 */
public class MainActivity extends Activity implements MqttService.MqttCallback {
    
    private static final String TAG = "SmartHomeMain";
    
    // UI Components
    private TextView tvConnectionStatus;
    private TextView tvSummary;
    private RecyclerView rvDevices;
    private Button btnAllOn, btnAllOff, btnRefresh;
    private Switch swAutoConnect;
    
    // Service
    private MqttService mqttService;
    private boolean serviceBound = false;
    
    // Devices
    private DeviceAdapter deviceAdapter;
    private List<Device> devices;
    
    // Service Connection
    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            MqttService.LocalBinder binder = (MqttService.LocalBinder) service;
            mqttService = binder.getService();
            mqttService.setCallback(MainActivity.this);
            serviceBound = true;
            
            Log.d(TAG, "MQTT Service connected");
            updateConnectionStatus();
            
            // Request initial status
            if (mqttService.isConnected()) {
                mqttService.publishMessage("smarthome/control", "status");
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            serviceBound = false;
            mqttService = null;
            Log.d(TAG, "MQTT Service disconnected");
        }
    };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        Log.d(TAG, "onCreate");
        
        initializeViews();
        initializeDevices();
        setupListeners();
        
        // Start MQTT Service
        Intent intent = new Intent(this, MqttService.class);
        startService(intent);
        bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }
    
    private void initializeViews() {
        tvConnectionStatus = findViewById(R.id.tv_connection_status);
        tvSummary = findViewById(R.id.tv_summary);
        rvDevices = findViewById(R.id.rv_devices);
        btnAllOn = findViewById(R.id.btn_all_on);
        btnAllOff = findViewById(R.id.btn_all_off);
        btnRefresh = findViewById(R.id.btn_refresh);
        swAutoConnect = findViewById(R.id.sw_auto_connect);
        
        // Setup RecyclerView
        rvDevices.setLayoutManager(new LinearLayoutManager(this));
    }
    
    private void initializeDevices() {
        devices = new ArrayList<>();
        
        // Kitchen Light (GPIO 10)
        devices.add(new Device(
            "kitchen_light",
            "Kitchen Light",
            "Light",
            DeviceType.LIGHT,
            10,
            false
        ));
        
        // Bedroom Light (GPIO 11)
        devices.add(new Device(
            "bed1_light",
            "Bedroom Light",
            "Light",
            DeviceType.LIGHT,
            11,
            false
        ));
        
        // Living Hall Light (GPIO 12)
        devices.add(new Device(
            "living_light",
            "Living Hall Light",
            "Light",
            DeviceType.LIGHT,
            12,
            false
        ));
        
        // Buzzer (GPIO 16)
        devices.add(new Device(
            "buzzer",
            "Buzzer",
            "Alert",
            DeviceType.BUZZER,
            16,
            false
        ));
        
        // Setup adapter
        deviceAdapter = new DeviceAdapter(devices, new DeviceAdapter.OnDeviceClickListener() {
            @Override
            public void onDeviceClick(Device device) {
                toggleDevice(device);
            }
        });
        
        rvDevices.setAdapter(deviceAdapter);
        updateSummary();
    }
    
    private void setupListeners() {
        btnAllOn.setOnClickListener(v -> controlAllDevices(true));
        btnAllOff.setOnClickListener(v -> controlAllDevices(false));
        btnRefresh.setOnClickListener(v -> refreshStatus());
        
        swAutoConnect.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (serviceBound && mqttService != null) {
                if (isChecked && !mqttService.isConnected()) {
                    mqttService.connect();
                } else if (!isChecked && mqttService.isConnected()) {
                    mqttService.disconnect();
                }
            }
        });
    }
    
    private void toggleDevice(Device device) {
        if (!serviceBound || mqttService == null || !mqttService.isConnected()) {
            Toast.makeText(this, "Not connected to MQTT", Toast.LENGTH_SHORT).show();
            return;
        }
        
        boolean newState = !device.isOn();
        String command = device.getId() + "_" + (newState ? "on" : "off");
        
        Log.d(TAG, "Sending command: " + command);
        
        if (mqttService.publishMessage("smarthome/control", command)) {
            // Optimistically update UI
            device.setOn(newState);
            deviceAdapter.notifyDataSetChanged();
            updateSummary();
            
            Toast.makeText(this, 
                device.getName() + " " + (newState ? "ON" : "OFF"), 
                Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this, "Failed to send command", Toast.LENGTH_SHORT).show();
        }
    }
    
    private void controlAllDevices(boolean turnOn) {
        if (!serviceBound || mqttService == null || !mqttService.isConnected()) {
            Toast.makeText(this, "Not connected to MQTT", Toast.LENGTH_SHORT).show();
            return;
        }
        
        String command = turnOn ? "all_on" : "all_off";
        
        if (mqttService.publishMessage("smarthome/control", command)) {
            // Update all lights (not buzzer)
            for (Device device : devices) {
                if (device.getType() == DeviceType.LIGHT) {
                    device.setOn(turnOn);
                }
            }
            deviceAdapter.notifyDataSetChanged();
            updateSummary();
            
            Toast.makeText(this, 
                "All lights " + (turnOn ? "ON" : "OFF"), 
                Toast.LENGTH_SHORT).show();
        }
    }
    
    private void refreshStatus() {
        if (!serviceBound || mqttService == null || !mqttService.isConnected()) {
            Toast.makeText(this, "Not connected to MQTT", Toast.LENGTH_SHORT).show();
            return;
        }
        
        mqttService.publishMessage("smarthome/control", "status");
        Toast.makeText(this, "Refreshing status...", Toast.LENGTH_SHORT).show();
    }
    
    private void updateSummary() {
        int onCount = 0;
        for (Device device : devices) {
            if (device.isOn()) {
                onCount++;
            }
        }
        tvSummary.setText(onCount + " devices ON");
    }
    
    private void updateConnectionStatus() {
        runOnUiThread(() -> {
            if (serviceBound && mqttService != null && mqttService.isConnected()) {
                tvConnectionStatus.setText("🟢 Connected");
                tvConnectionStatus.setTextColor(0xFF22d18b);
                swAutoConnect.setChecked(true);
            } else {
                tvConnectionStatus.setText("🔴 Disconnected");
                tvConnectionStatus.setTextColor(0xFFff4444);
                swAutoConnect.setChecked(false);
            }
        });
    }
    
    // MqttService.MqttCallback Implementation
    
    @Override
    public void onConnectionStatusChanged(boolean connected) {
        Log.d(TAG, "Connection status changed: " + connected);
        updateConnectionStatus();
        
        if (connected) {
            runOnUiThread(() -> 
                Toast.makeText(this, "Connected to MQTT", Toast.LENGTH_SHORT).show()
            );
        }
    }
    
    @Override
    public void onMessageReceived(String topic, String message) {
        Log.d(TAG, "Message received - Topic: " + topic + ", Message: " + message);
        
        if (topic.equals("smarthome/status")) {
            parseStatusMessage(message);
        }
    }
    
    private void parseStatusMessage(String jsonMessage) {
        try {
            JSONObject json = new JSONObject(jsonMessage);
            
            // Update device states
            for (Device device : devices) {
                if (json.has(device.getId())) {
                    String state = json.getString(device.getId());
                    device.setOn(state.equalsIgnoreCase("on"));
                }
            }
            
            runOnUiThread(() -> {
                deviceAdapter.notifyDataSetChanged();
                updateSummary();
            });
            
        } catch (Exception e) {
            Log.e(TAG, "Error parsing status message", e);
        }
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (serviceBound) {
            unbindService(serviceConnection);
            serviceBound = false;
        }
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        updateConnectionStatus();
        if (serviceBound && mqttService != null && mqttService.isConnected()) {
            refreshStatus();
        }
    }
}
