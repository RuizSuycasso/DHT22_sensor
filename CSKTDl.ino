#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>  
#include "DHT.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "An đính đá"
#define WIFI_PASSWORD "97148622"

#define API_KEY "AIzaSyB-bc6ycEN1PJvuH_O1kcaoSgYJhJO6e3c"

#define DATABASE_URL "https://btl-csdl-default-rtdb.asia-southeast1.firebasedatabase.app/" 

#define DHTPIN 4       
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiServer server(80); 

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
 
  server.begin();
  Serial.println("HTTP Server started.");
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Sign-Up OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase Sign-Up Error: %s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  String ipAddress = WiFi.localIP().toString();
  if (Firebase.RTDB.setString(&fbdo, "device/ip", ipAddress)) {
    Serial.print("Device IP uploaded successfully: ");
    Serial.println(ipAddress);
  } else {
    Serial.println("Failed to upload Device IP!");
    Serial.println(fbdo.errorReason());
  }
}

void loop() {
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected.");
    String request = client.readStringUntil('\r'); 
    Serial.println("Request: " + request);

   

    delay(10);
    client.stop();
    Serial.println("Client disconnected.");
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    if (Firebase.RTDB.setFloat(&fbdo, "sensor/temperature", temperature)) {
      Serial.println("Temperature uploaded successfully!");
    } else {
      Serial.println("Failed to upload temperature!");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "sensor/humidity", humidity)) {
      Serial.println("Humidity uploaded successfully!");
    } else {
      Serial.println("Failed to upload humidity!");
      Serial.println(fbdo.errorReason());
    }
  }
}