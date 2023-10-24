#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Mourning to Dancing";
const char* password = "throughchristalone";
const char* serverAddress = "http://dwell.local/post";


int force = 0;
  HTTPClient http;

void getForce(){

    int sum = 0;

for (int i = 0; i<50; i++){
    sum = sum + analogRead(33);
    delay(12);
}
force = sum/50;
Serial.println((int)sum/50);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Send POST request

}

void sendData(int warmLed){
    
  // Specify the content type and data to send
  http.begin(serverAddress);
  http.addHeader("Content-Type", "application/json");

  // Create a JSON payload
  String jsonPayload = "{\"warmLed\": " + String(warmLed) + ", \"whiteLed\": 0}";

  // Send the POST request with the payload
  int httpCode = http.POST(jsonPayload);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("HTTP response: " + response);
  } else {
    Serial.println("HTTP request failed");
  }

  http.end();
}

void loop() {
  getForce();
  if (force < 3500) {
Serial.println(int(float(force) / 4095 * 100));

    sendData(int(float(force) / 4095 * 100));
}
}
