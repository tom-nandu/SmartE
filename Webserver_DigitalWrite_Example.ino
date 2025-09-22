
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include "IIOT_Network.h"  // Contains rootCA, deviceCert, privateKey

// Wi-Fi credentials
const char* ssid = "Charlie";
const char* password = "logu9769";
const char* username = "admin";
const char* pwd = "1234";


// AWS IoT certificates
const char*  deviceCert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWTCCAkGgAwIBAgIUZXryht79SAwndTL68V8nSEwS/RcwDQYJKoZIhvcNAQEL\n" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDkyMDA3NDYy\n" \
"N1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMIVWFw0jLiBSHd4y4TG\n" \
"DXx0q6I7G2XXbEJdz1djCi5YEYfvVs7IZzWwWNgwJ67A+yQGP1pzUN38k9crFf+f\n" \
"ymU2R1b2ubWH7w2SCrGbpSaa5mtXbvFhbNO7lvgFtrFHCndmBX0gtzmUMzXKkau8\n" \
"Q3KZosBLjczyBUBks+BNt7ywmiQzsCfZbL4GEpRAWCn9+XOrNT2ijF/zLSqOGu1U\n" \
"SuaGKqDVK68LRpKt69etddYSY98GSsXAjGaeJaYoVBJMKXmnHqqKb0EZkQKkUvPs\n" \
"2WZpHdog6yxwEH2tDM+JgCZQ6iu8mstVtvgPPN8voD6HrkDJRYJ0Q9e7zoc2velQ\n" \
"oasCAwEAAaNgMF4wHwYDVR0jBBgwFoAUYnsfjF9yc+rOgQGGZBlwtBolQw8wHQYD\n" \
"VR0OBBYEFC0pm2RQYR1rT7lNine+VENbmS65MAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCFhPwhAfKpklEIuvYNINMhQTx1\n" \
"TxEAo6PZwE7s93D610xguTuQIw3EAaIjvwJZ0ouDrPAyqsdv+aDtiFfrPjJOw70h\n" \
"D1fIPRIubknFlTBNw+IfYhDkJNqwhsHIFJdlFT6H5/1vwHkOGVGJ6HskS7Y1Ml7H\n" \
"ZBbEHEtNJSOlgECj72Klg78/RZTAaODfqMLaMHra1tXYNSemYxf7cabI7qkQjSvx\n" \
"ct6Y/6sPjgWqYQCs8vRqOjOkbVxBM7j9VojK51K3ie4RP93T/Klq8ukQqKK9VmFp\n" \
"zRtadA7RALfOIIJwp/aU9q8zF4PZp8EUA266AdSQx+U2/k4V76FWnRpJATVM\n" \
"-----END CERTIFICATE-----\n";

const char* rootCA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

const char* privateKey = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEAwhVYXDSMuIFId3jLhMYNfHSrojsbZddsQl3PV2MKLlgRh+9W\n" \
"zshnNbBY2DAnrsD7JAY/WnNQ3fyT1ysV/5/KZTZHVva5tYfvDZIKsZulJprma1du\n" \
"8WFs07uW+AW2sUcKd2YFfSC3OZQzNcqRq7xDcpmiwEuNzPIFQGSz4E23vLCaJDOw\n" \
"J9lsvgYSlEBYKf35c6s1PaKMX/MtKo4a7VRK5oYqoNUrrwtGkq3r16111hJj3wZK\n" \
"xcCMZp4lpihUEkwpeaceqopvQRmRAqRS8+zZZmkd2iDrLHAQfa0Mz4mAJlDqK7ya\n" \
"y1W2+A883y+gPoeuQMlFgnRD17vOhza96VChqwIDAQABAoIBAGhd5hgfQhTtoLRu\n" \
"RrLtdc8ZRjqFImSPhW3i2F6bqZvmS/cXY0zrQ2UBoaUPsCcvK13h5iuTm0bkTfQE\n" \
"/V1q5gGrQI2pO5A8uvZHNGzxz8uEvKe4arZut4DcfYB+QhHuq9gtCoKRFzZlVWx1\n" \
"tAPvxy1XJMfXWTd+ZgiMnpXd9RIHzak2uxRWOODHHG0pzlyx3+tIc1mK87xUlOYD\n" \
"YM83RaRd8HT0f6JK8oZflT+Sa1vhknyQOkkXf7G7ZRDwPXhtt8hYMwbtrtMnLal7\n" \
"WjMXdbLTzeMQDuTEfdSgyA70WlkEgyPBTAGVEGcdqL1d7S0EQ95tK3soQKAsrdSL\n" \
"QD2qzEECgYEA6yB/pevg8XNiHacPCsd+DwR8Wm/XiK4wQeDOylxXSKLFI4wj6UDR\n" \
"p6zxOYdFyLqcVkEZRdhd/EMk/cUK98p76nT0nh2UCINzVTBCgfYbTXwSWnGwWDPX\n" \
"LFar4mSIxa+Pmr4KrtV+I7CV8pwii6NgVT7a7Z6Av2yI+O/D/+WasxECgYEA01AW\n" \
"262sOq2BH8SZbcOpeeuOOiqgIfYvwT1Huk/uSR71E3ARPhS2jnxORWNsuahKO9Rx\n" \
"1/tzJgBwJH3r9j6jONQz2b6+xmNMh/kxoQB6yxxRuzyjSsZdC6kkm/x4TMRBotFl\n" \
"+G7/s7vUPLbZkwmWBDdmyDgv7UAkbCM63IRSEPsCgYAUdQFmKD2sBEP3HH/cijrX\n" \
"h6ZiH/T6uV0Nfke/p4UYlsDcNOcy9ibHle5u+OwDaOkNQCC4yPh0aEshoTExGfbq\n" \
"ET7vOteUXgt8z/QZZpY9iZv3LokPQ4NnIitWSTT1Li8qYXqAd6a/6C9Dqn3+9Dn/\n" \
"cLaPkgjVntXUAthkvU9WYQKBgGK01YiWDo52P4usVRL9w6uMyaoIATb7/YChdGDm\n" \
"N2N2j3od7h/2ovPZwuIMuFvc7ZgW+3qtHTOSHtAc9dzXk3zXOsUqoYigF76oO3N+\n" \
"et1nhBIXGBu0nv/0aYJno3YQcqxwbdZnWxAE2/XkX6ucXbPogR1jiZekT58nsbYS\n" \
"JReDAoGAZLMsTnaUbcafc4UGjNuSwx456zf7CSdWywm7vqzNEEI9av91s1AtrIZX\n" \
"A8lIi9ErSzI0u21OZtMJd++LLghXTMfiE1EYcicrm4d+bg1cg/poPIN19X75Y7Yd\n" \
"dOR6lzc4Mt+gAnfqhwCihFIss7bHIDy1eo2zG9r9j8tGf1T+i0w=\n" \
"-----END RSA PRIVATE KEY-----\n";


// AWS IoT endpoint
const char* awsEndpoint = "a2xlftksgunodq-ats.iot.us-east-1.amazonaws.com";

// LED setup
#define NUM_LEDS 2
CRGB leds[NUM_LEDS];
#define STATUS_LED (leds[0])
#define NETWORK_LED (leds[1])
CRGB xledColor;

// Global objects
WiFiClientSecure net;
PubSubClient client(net);
WebServer server(80);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
xQueueHandle queueHandleIdWsLed;

class ws2812b_led {
public:
    void init();
};

void wsledTask(void* pvParameters) {
    FastLED.addLeds<WS2812B, 5, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(10);
    STATUS_LED = CRGB::Green;
    FastLED.show();

    CRGB ledColor;
    while (1) {
        if (xQueueReceive(queueHandleIdWsLed, &ledColor, portMAX_DELAY) == pdPASS) {
            STATUS_LED = ledColor;
            Serial.printf("LED updated: R%d G%d B%d\n", ledColor.r, ledColor.g, ledColor.b);
            FastLED.show();
        }
        delay(100);
    }
}

void ws2812b_led::init() {
    queueHandleIdWsLed = xQueueCreate(1, sizeof(CRGB));
    xTaskCreate(wsledTask, "wsledTask", 1024 * 3, NULL, 1, NULL);
}

void updateOLEDDisplay(CRGB color, const char* name) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.printf("Color: R%d G%d B%d\n", color.r, color.g, color.b);
    display.printf("User: %s", name);
    display.display();
}

void publishLEDStatus(const char* name, CRGB color) {
    String payload = "{\"color\":\"" + String(color.r) + "," + String(color.g) + "," + String(color.b) + "\",\"name\":\"" + String(name) + "\"}";
    client.publish("esp32/status", payload.c_str());
}

void handleRoot() {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>ESP32 LED Dashboard</title>
        <style>
            body { font-family: Arial; text-align: center; margin-top: 40px; }
            button { padding: 10px 20px; margin: 5px; font-size: 16px; }
            #statusBox { margin-top: 20px; font-size: 18px; }
        </style>
        <script>
            function sendCommand(cmd) {
                fetch(cmd, {
                    headers: {
                        'Authorization': 'Basic ' + btoa('admin:1234')
                    }
                })
                .then(response => response.text())
                .then(data => {
                    document.getElementById('response').innerText = data;
                    updateStatus();
                })
                .catch(err => console.error("Fetch error:", err));
            }

            function updateStatus() {
                fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('ledColor').innerText = data.led;
                    document.getElementById('ipAddr').innerText = data.ip;
                });
            }

            setInterval(updateStatus, 2000);
            window.onload = updateStatus;
        </script>
    </head>
    <body>
        <h1>ESP32 IIoT LED Control</h1>
        <img src='https://i.imgur.com/3ZQ3Z2L.png' alt='Home Image' width='150'><br><br>
        <h2>Color Control</h2>
        <button onclick="sendCommand('/green')">Green</button>
        <button onclick="sendCommand('/red')">Red</button>
        <button onclick="sendCommand('/blue')">Blue</button><br><br>
        <h2>Power Toggle</h2>
        <button onclick="sendCommand('/on')">Turn ON</button>
        <button onclick="sendCommand('/off')">Turn OFF</button>
        <div id="statusBox">
            <p><strong>LED Color:</strong> <span id="ledColor">---</span></p>
            <p><strong>Device IP:</strong> <span id="ipAddr">---</span></p>
            <p><strong>Response:</strong> <span id="response">---</span></p>
        </div>
    </body>
    </html>
    )rawliteral";

    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    ws2812b_led led;
    led.init();

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp32")) {
        Serial.println("MDNS responder started");
    }

    net.setCACert(rootCA);
    net.setCertificate(deviceCert);
    net.setPrivateKey(privateKey);

    client.setServer(awsEndpoint, 8883);
    client.setCallback([](char* topic, byte* payload, unsigned int length) {
        String cmd = String((char*)payload).substring(0, length);
        Serial.println("Received MQTT command: " + cmd);

        if (cmd == "green") xledColor = CRGB::Green;
        else if (cmd == "red") xledColor = CRGB::Red;
        else if (cmd == "blue") xledColor = CRGB::Blue;
        else return;

        xQueueSend(queueHandleIdWsLed, &xledColor, 0);
        updateOLEDDisplay(xledColor, cmd.c_str());
    });

    if (client.connect("ESP_32")) {
        Serial.println("Connected to AWS IoT!");
        client.publish("esp32/topic", "Hello from ESP32");
        client.subscribe("esp32/commands");
    } else {
        Serial.println("AWS IoT connection failed");
        Serial.print("MQTT connect state: ");
        Serial.println(client.state());
    }

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();

    server.on("/", handleRoot);
    server.on("/favicon.ico", HTTP_GET, []() { server.send(204); });

    server.on("/green", HTTP_GET, []() {
        Serial.println("HTTP /green triggered");
        if (!server.authenticate(username, pwd)) return server.requestAuthentication();
        xledColor = CRGB::Green;
        xQueueSend(queueHandleIdWsLed, &xledColor, 0);
        updateOLEDDisplay(xledColor, "GREEN");
        publishLEDStatus("GREEN", xledColor);
        server.send(200, "text/plain", "LED set to green");
    });

    server.on("/red", HTTP_GET, []() {
        Serial.println("HTTP /red triggered");
        if (!server.authenticate(username, pwd)) return server.requestAuthentication();
        xledColor = CRGB::Red;
        xQueueSend(queueHandleIdWsLed, &xledColor, 0);
        updateOLEDDisplay(xledColor, "RED");
        publishLEDStatus("RED", xledColor);
        server.send(200, "text/plain", "LED set to red");
    });

    server.on("/blue", HTTP_GET, []() {
        Serial.println("HTTP /blue triggered");
        if (!server.authenticate(username, pwd)) return server.requestAuthentication();
        xledColor = CRGB::Blue;
        xQueueSend(queueHandleIdWsLed, &xledColor, 0);
        updateOLEDDisplay(xledColor, "BLUE");
        publishLEDStatus("BLUE", xledColor);
        server.send(200, "text/plain", "LED set to blue");
    });


    server.on("/on", HTTP_GET, []() {
        Serial.println("HTTP /on triggered");
        if (!server.authenticate(username, pwd)) return server.requestAuthentication();
        FastLED.setBrightness(50);
        FastLED.show();
        server.send(200, "text/plain", "LED turned ON");
    });

    server.on("/off", HTTP_GET, []() {
        Serial.println("HTTP /off triggered");
        if (!server.authenticate(username, pwd)) return server.requestAuthentication();
        FastLED.setBrightness(0);
        FastLED.show();
        server.send(200, "text/plain", "LED turned OFF");
    });

    server.on("/status", HTTP_GET, []() {
        String json = "{";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        json += "\"led\":\"" + String(xledColor.r) + "," + String(xledColor.g) + "," + String(xledColor.b) + "\"";
        json += "}";
        server.send(200, "application/json", json);
    });

    server.onNotFound([]() {
        server.send(404, "text/plain", "404: Not Found");
    });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();

    static unsigned long lastReconnectAttempt = 0;

    if (!client.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            Serial.println("Attempting MQTT reconnect...");
            client.setKeepAlive(60);

            if (client.connect("ESP_32")) {
                Serial.println("Reconnected to AWS IoT!");
                client.subscribe("esp32/commands");
            } else {
                Serial.print("Reconnect failed, state: ");
                Serial.println(client.state());
            }
        }
    } else {
        client.loop();
    }

    // Update OLED with MQTT status
    display.fillRect(0, 50, 128, 14, SSD1306_BLACK); // Clear bottom line
    display.setCursor(0, 50);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.printf("MQTT: %s", client.connected() ? "Connected" : "Disconnected");
    display.display();

    delay(10);
}

    