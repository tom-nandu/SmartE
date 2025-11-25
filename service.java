package com.android.smarthome;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.*;

/**
 * Background service for maintaining MQTT connection
 * Handles connection, subscription, publishing, and reconnection
 */
public class MqttService extends Service {
    
    private static final String TAG = "MqttService";
    
    // MQTT Configuration
    private static final String BROKER_URL = "tcp://broker.hivemq.com:1883";
    private static final String CLIENT_ID = "AndroidSmartHome_";
    private static final String TOPIC_CONTROL = "smarthome/control";
    private static final String TOPIC_STATUS = "smarthome/status";
    
    // Notification
    private static final int NOTIFICATION_ID = 1001;
    private static final String CHANNEL_ID = "smart_home_service";
    
    // MQTT Client
    private MqttAndroidClient mqttClient;
    private MqttCallback callback;
    private boolean isConnected = false;
    
    // Binder for Activity
    private final IBinder binder = new LocalBinder();
    
    public class LocalBinder extends Binder {
        MqttService getService() {
            return MqttService.this;
        }
    }
    
    public interface MqttCallback {
        void onConnectionStatusChanged(boolean connected);
        void onMessageReceived(String topic, String message);
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "Service created");
        
        createNotificationChannel();
        startForeground(NOTIFICATION_ID, createNotification("Initializing..."));
        
        initializeMqttClient();
        connect();
    }
    
    private void createNotificationChannel() {
        NotificationChannel channel = new NotificationChannel(
            CHANNEL_ID,
            "Smart Home Service",
            NotificationManager.IMPORTANCE_LOW
        );
        channel.setDescription("Maintains connection to smart home devices");
        
        NotificationManager manager = getSystemService(NotificationManager.class);
        if (manager != null) {
            manager.createNotificationChannel(channel);
        }
    }
    
    private Notification createNotification(String status) {
        Intent intent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(
            this, 0, intent, PendingIntent.FLAG_IMMUTABLE
        );
        
        return new Notification.Builder(this, CHANNEL_ID)
            .setContentTitle("Smart Home Control")
            .setContentText(status)
            .setSmallIcon(android.R.drawable.ic_dialog_info)
            .setContentIntent(pendingIntent)
            .build();
    }
    
    private void initializeMqttClient() {
        String clientId = CLIENT_ID + System.currentTimeMillis();
        mqttClient = new MqttAndroidClient(
            getApplicationContext(),
            BROKER_URL,
            clientId
        );
        
        mqttClient.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                Log.d(TAG, "Connection complete. Reconnect: " + reconnect);
                isConnected = true;
                updateNotification("Connected");
                
                if (callback != null) {
                    callback.onConnectionStatusChanged(true);
                }
                
                // Subscribe to topics
                subscribeToTopics();
            }

            @Override
            public void connectionLost(Throwable cause) {
                Log.e(TAG, "Connection lost", cause);
                isConnected = false;
                updateNotification("Disconnected - Reconnecting...");
                
                if (callback != null) {
                    callback.onConnectionStatusChanged(false);
                }
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) {
                String payload = new String(message.getPayload());
                Log.d(TAG, "Message arrived - Topic: " + topic + ", Message: " + payload);
                
                if (callback != null) {
                    callback.onMessageReceived(topic, payload);
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                Log.d(TAG, "Message delivered");
            }
        });
    }
    
    public void connect() {
        if (mqttClient == null || mqttClient.isConnected()) {
            return;
        }
        
        MqttConnectOptions options = new MqttConnectOptions();
        options.setAutomaticReconnect(true);
        options.setCleanSession(true);
        options.setConnectionTimeout(10);
        options.setKeepAliveInterval(60);
        
        try {
            Log.d(TAG, "Connecting to MQTT broker...");
            updateNotification("Connecting...");
            
            mqttClient.connect(options, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d(TAG, "Connected successfully");
                    isConnected = true;
                    updateNotification("Connected");
                    
                    if (callback != null) {
                        callback.onConnectionStatusChanged(true);
                    }
                    
                    subscribeToTopics();
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.e(TAG, "Connection failed", exception);
                    isConnected = false;
                    updateNotification("Connection failed");
                    
                    if (callback != null) {
                        callback.onConnectionStatusChanged(false);
                    }
                }
            });
            
        } catch (MqttException e) {
            Log.e(TAG, "Error connecting", e);
        }
    }
    
    private void subscribeToTopics() {
        try {
            mqttClient.subscribe(TOPIC_STATUS, 1, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d(TAG, "Subscribed to: " + TOPIC_STATUS);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.e(TAG, "Subscription failed", exception);
                }
            });
            
        } catch (MqttException e) {
            Log.e(TAG, "Error subscribing", e);
        }
    }
    
    public boolean publishMessage(String topic, String message) {
        if (!isConnected || mqttClient == null) {
            Log.w(TAG, "Cannot publish - not connected");
            return false;
        }
        
        try {
            MqttMessage mqttMessage = new MqttMessage(message.getBytes());
            mqttMessage.setQos(1);
            mqttMessage.setRetained(false);
            
            mqttClient.publish(topic, mqttMessage, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d(TAG, "Published: " + message + " to " + topic);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.e(TAG, "Publish failed", exception);
                }
            });
            
            return true;
            
        } catch (MqttException e) {
            Log.e(TAG, "Error publishing message", e);
            return false;
        }
    }
    
    public void disconnect() {
        if (mqttClient != null && mqttClient.isConnected()) {
            try {
                mqttClient.disconnect();
                isConnected = false;
                updateNotification("Disconnected");
                
                if (callback != null) {
                    callback.onConnectionStatusChanged(false);
                }
                
            } catch (MqttException e) {
                Log.e(TAG, "Error disconnecting", e);
            }
        }
    }
    
    public boolean isConnected() {
        return isConnected && mqttClient != null && mqttClient.isConnected();
    }
    
    public void setCallback(MqttCallback callback) {
        this.callback = callback;
    }
    
    private void updateNotification(String status) {
        NotificationManager manager = getSystemService(NotificationManager.class);
        if (manager != null) {
            manager.notify(NOTIFICATION_ID, createNotification(status));
        }
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "Service started");
        return START_STICKY;
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "Service destroyed");
        disconnect();
    }
}
