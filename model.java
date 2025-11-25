// ============================================
// Device.java - Device Model
// ============================================
package com.android.smarthome;

public class Device {
    private String id;
    private String name;
    private String label;
    private DeviceType type;
    private int gpioPin;
    private boolean isOn;
    
    public Device(String id, String name, String label, DeviceType type, int gpioPin, boolean isOn) {
        this.id = id;
        this.name = name;
        this.label = label;
        this.type = type;
        this.gpioPin = gpioPin;
        this.isOn = isOn;
    }
    
    public String getId() { return id; }
    public String getName() { return name; }
    public String getLabel() { return label; }
    public DeviceType getType() { return type; }
    public int getGpioPin() { return gpioPin; }
    public boolean isOn() { return isOn; }
    
    public void setOn(boolean on) { isOn = on; }
    
    public String getIcon() {
        switch (type) {
            case LIGHT: return "💡";
            case FAN: return "🌀";
            case BUZZER: return "🔔";
            default: return "⚡";
        }
    }
    
    public int getColor() {
        if (!isOn) return 0xFF394651;
        
        switch (type) {
            case LIGHT: return 0xFF22d18b;
            case FAN: return 0xFF7f8cff;
            case BUZZER: return 0xFFff3333;
            default: return 0xFF00ffff;
        }
    }
}

// ============================================
// DeviceType.java - Enum for device types
// ============================================
package com.android.smarthome;

public enum DeviceType {
    LIGHT,
    FAN,
    BUZZER,
    SENSOR
}

// ============================================
// DeviceAdapter.java - RecyclerView Adapter
// ============================================
package com.android.smarthome;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Switch;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.List;

public class DeviceAdapter extends RecyclerView.Adapter<DeviceAdapter.DeviceViewHolder> {
    
    private List<Device> devices;
    private OnDeviceClickListener listener;
    
    public interface OnDeviceClickListener {
        void onDeviceClick(Device device);
    }
    
    public DeviceAdapter(List<Device> devices, OnDeviceClickListener listener) {
        this.devices = devices;
        this.listener = listener;
    }
    
    @NonNull
    @Override
    public DeviceViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
            .inflate(R.layout.device_item, parent, false);
        return new DeviceViewHolder(view);
    }
    
    @Override
    public void onBindViewHolder(@NonNull DeviceViewHolder holder, int position) {
        Device device = devices.get(position);
        holder.bind(device, listener);
    }
    
    @Override
    public int getItemCount() {
        return devices.size();
    }
    
    static class DeviceViewHolder extends RecyclerView.ViewHolder {
        
        private TextView tvIcon;
        private TextView tvName;
        private TextView tvLabel;
        private TextView tvGpio;
        private Switch swState;
        private View indicator;
        
        public DeviceViewHolder(@NonNull View itemView) {
            super(itemView);
            
            tvIcon = itemView.findViewById(R.id.tv_icon);
            tvName = itemView.findViewById(R.id.tv_name);
            tvLabel = itemView.findViewById(R.id.tv_label);
            tvGpio = itemView.findViewById(R.id.tv_gpio);
            swState = itemView.findViewById(R.id.sw_state);
            indicator = itemView.findViewById(R.id.indicator);
        }
        
        public void bind(Device device, OnDeviceClickListener listener) {
            tvIcon.setText(device.getIcon());
            tvName.setText(device.getName());
            tvLabel.setText(device.getLabel());
            tvGpio.setText("GPIO " + device.getGpioPin());
            swState.setChecked(device.isOn());
            
            // Update indicator color
            indicator.setBackgroundColor(device.getColor());
            
            // Set click listener on switch
            swState.setOnClickListener(v -> {
                if (listener != null) {
                    listener.onDeviceClick(device);
                }
            });
            
            // Set click listener on entire item
            itemView.setOnClickListener(v -> {
                if (listener != null) {
                    listener.onDeviceClick(device);
                }
            });
        }
    }
}

// ============================================
// QuickSettingsTile.java - Quick Settings Integration
// ============================================
package com.android.smarthome;

import android.graphics.drawable.Icon;
import android.service.quicksettings.Tile;
import android.service.quicksettings.TileService;
import android.content.Intent;
import android.util.Log;

/**
 * Quick Settings Tile for controlling lights
 * Swipe down notification shade -> Click tile to toggle all lights
 */
public class QuickSettingsTile extends TileService {
    
    private static final String TAG = "SmartHomeTile";
    private boolean lightsOn = false;
    
    @Override
    public void onStartListening() {
        super.onStartListening();
        updateTile();
    }
    
    @Override
    public void onClick() {
        super.onClick();
        
        lightsOn = !lightsOn;
        
        // Send broadcast to toggle lights
        Intent intent = new Intent("com.android.smarthome.TOGGLE_ALL");
        intent.putExtra("state", lightsOn);
        sendBroadcast(intent);
        
        updateTile();
        
        Log.d(TAG, "Toggled lights: " + (lightsOn ? "ON" : "OFF"));
    }
    
    private void updateTile() {
        Tile tile = getQsTile();
        if (tile != null) {
            tile.setState(lightsOn ? Tile.STATE_ACTIVE : Tile.STATE_INACTIVE);
            tile.setLabel(lightsOn ? "Lights ON" : "Lights OFF");
            tile.updateTile();
        }
    }
}

// ============================================
// BootReceiver.java - Auto-start on boot
// ============================================
package com.android.smarthome;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * Starts MQTT service on device boot
 */
public class BootReceiver extends BroadcastReceiver {
    
    private static final String TAG = "SmartHomeBootReceiver";
    
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            Log.d(TAG, "Boot completed - starting MQTT service");
            
            Intent serviceIntent = new Intent(context, MqttService.class);
            context.startForegroundService(serviceIntent);
        }
    }
}
