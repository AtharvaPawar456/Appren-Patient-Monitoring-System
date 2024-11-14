// Include necessary libraries
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char *ssid = "MAHAKAL";
const char *password = "sateri456";

// Server URL
const char *serverUrl = "https://api.thingspeak.com/update?api_key=VFEZS0RACY1RKYBH";

// DHT sensor configuration
#define DHTPIN 4       // Pin where the DHT11 data pin is connected
#define DHTTYPE DHT11  // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);  // Initialize the DHT sensor

// Sensor pins
const int pulsePin = 32;   // Heart pulse sensor pin
const int buzzerPin = 16;  // Buzzer pin
const int floatPin = 17;   // Float sensor pin for urine bag
const int tactilePin = 18; // Tactile sensor pin for saline bag
const int ledPin = 2;      // Built-in LED on ESP32 (usually GPIO 2)

// Threshold values
int signalVal;                      // For pulse signal
const int pulseThreshold = 98;   // Pulse threshold value
const int tempThreshold = 40;    // Temperature threshold value

// ThingSpeak fields
int field1Val = 0;
int field3Val = 0;
int field4Val = 0;
int field5Val = 0;

// Function to make a GET request to ThingSpeak
void makeGetRequest() {
  HTTPClient http;
  String url = String(serverUrl) +
               "&field1=" + field1Val +
               "&field2=" + "0" +
               "&field3=" + field3Val +
               "&field4=" + field4Val +
               "&field5=" + field5Val;

  Serial.println("Sending GET request to: " + url);
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.printf("HTTP response code: %d\n", httpCode);
    Serial.println("Server response: " + http.getString());
  } else {
    Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  pinMode(ledPin, OUTPUT);

  // WiFi connection setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  digitalWrite(ledPin, LOW);
  Serial.println("Connected to WiFi");

  // Initialize pin modes
  pinMode(pulsePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(tactilePin, INPUT_PULLUP);
  pinMode(floatPin, INPUT_PULLUP);

  Serial.println("System Initialized");
}

void loop() {
  Serial.println("\n--------------------------------------\n");

  // Read DHT sensor values
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: ");Serial.println(humidity);
  Serial.print("Temperature: ");Serial.println(tempC);
  field1Val = tempC;

  // Simulate pulse reading
  signalVal = random(58, 101);
  Serial.print("Pulse Signal: ");Serial.println(signalVal);
  field3Val = signalVal;

  // Display pulse status
  Serial.println(signalVal > pulseThreshold ? "Pulse: HIGH" : "Pulse: LOW");

  // Read sensor states
  int tactileState = digitalRead(tactilePin); // LOW when saline bag is low
  int floatState = digitalRead(floatPin);     // LOW when urine bag is full
  Serial.println(tactileState == LOW ? "Saline Bag: LOW" : "Saline Bag: OK");
  Serial.println(floatState == HIGH ? "Urine Bag: FULL" : "Urine Bag: OK");

  // Temperature threshold check
  digitalWrite(buzzerPin, tempC > tempThreshold ? HIGH : LOW);
  if (tempC > tempThreshold) {
    Serial.println("Warning: Temperature above threshold!");
  }

  // Check saline bag status (tactile sensor)
  if (tactileState == LOW) {
    digitalWrite(buzzerPin, HIGH);
    Serial.println("Warning: Saline bag low!");
    field4Val = 1;
  } else {
    field4Val = 0;
  }

  // Check urine bag status (float sensor)
  if (floatState == HIGH) {
    digitalWrite(buzzerPin, HIGH);
    Serial.println("Warning: Urine Bag Full!");
    field5Val = 1;
  } else {
    field5Val = 0;
  }

  // Send data to ThingSpeak
  digitalWrite(ledPin, HIGH);delay(500);digitalWrite(ledPin, LOW);
  digitalWrite(ledPin, HIGH);delay(500);digitalWrite(ledPin, LOW);
  makeGetRequest();
  
  // Delay to avoid serial spamming
  delay(10000);  // 10-second delay
}
