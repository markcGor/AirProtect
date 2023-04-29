// Include necessary libraries
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP8266WiFi.h>

// Define constants for WiFi and Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL6IyOunn0P"
#define BLYNK_TEMPLATE_NAME "AirProtect"
#define BLYNK_AUTH_TOKEN "E1hftf5QyCEJmso05on40wIG516t1fMC"
#define WIFI_SSID "CHAN HOME 2.4"
#define WIFI_PASS "Kk26602660"

// Define constants for sensor types and GPIO pins
#define DHTTYPE DHT11
#define DHT11_GPIO 5
#define SCL_GPIO 14
#define SDA_GPIO 2
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
#define MQ135_GPIO A0

// Define constants for LED GPIO pins
#define GREEN_LED_GPIO 12
#define YELLOW_LED_GPIO 13
#define RED_LED_GPIO 15

// Declare global variables
unsigned long time_DHT = 0;
unsigned long time_OLED = 0;
unsigned long time_MQ135 = 0;
int temp = 0;
int hum = 0;
int air_quality = 0;

// Calibration factor for MQ135 sensor
float calibrationFactor = 0.19; // Change this value based on your sensor and environment

// Initialize sensor objects
DHT dht(DHT11_GPIO, DHTTYPE);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// Setup function
void setup() {
  // Initialize serial communication, sensors, display, and LEDs
  Serial.begin(115200);
  dht.begin();
  Wire.begin(SDA_GPIO, SCL_GPIO);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  pinMode(GREEN_LED_GPIO, OUTPUT);
  pinMode(YELLOW_LED_GPIO, OUTPUT);
  pinMode(RED_LED_GPIO, OUTPUT);

  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

// Main loop function
void loop() {
  // Run Blynk and call sensor update functions periodically
  Blynk.run();

  if (millis() >= time_DHT + 1000) {
    time_DHT += 1000;
    measureDHT();
  }

  if (millis() >= time_OLED + 1000) {
    time_OLED += 1000;
    refreshOLED();
  }

  if (millis() >= time_MQ135 + 1000) {
    time_MQ135 += 1000;
    measureMQ135();
    updateLEDs(getAQLevel(air_quality));

  }
}

// Function prototypes
void measureDHT() {
  temp = dht.readTemperature() + 0.5;
  hum = dht.readHumidity() + 0.5;

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
 
}

// Function prototypes
int convertAQ(int rawValue) {
  int aq = rawValue / calibrationFactor - 3000;
  return aq;
}

// Function prototypes
void measureMQ135() {
  int rawValue = analogRead(MQ135_GPIO);
  Serial.print("Raw value: ");
  Serial.println(rawValue);
  air_quality = convertAQ(rawValue);
  Serial.print("Air quality: ");
  Serial.println(air_quality);
  Blynk.virtualWrite(V2, air_quality);

  // Send air quality level to Blynk
  sendAQLevelToBlynk(air_quality);
}

// Function prototypes
String getAQLevel(int aq) {
  if (aq < 800) {
    return "Good";
  } else if (aq >= 800 && aq <= 2000) {
    return "Medium";
  } else {
    return "Bad";
  }
}

// Function prototypes
void updateLEDs(String level) {
  if (level == "Good") {
    digitalWrite(GREEN_LED_GPIO, HIGH);
    digitalWrite(YELLOW_LED_GPIO, LOW);
    digitalWrite(RED_LED_GPIO, LOW);
  } else if (level == "Medium") {
    digitalWrite(GREEN_LED_GPIO, LOW);
    digitalWrite(YELLOW_LED_GPIO, HIGH);
    digitalWrite(RED_LED_GPIO, LOW);
  } else if (level == "Bad") {
    digitalWrite(GREEN_LED_GPIO, LOW);
    digitalWrite(YELLOW_LED_GPIO, LOW);
    digitalWrite(RED_LED_GPIO, HIGH);
  }
}

// Function prototypes
void sendAQLevelToBlynk(int aq) {
  String level = getAQLevel(aq);
  Blynk.virtualWrite(V3, level);
}

// Function prototypes
void refreshOLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("T: ");
  display.print(temp);
  display.print("C  ");
  display.setCursor(0, 20);
  display.print("H: ");
  display.print(hum);
  display.print("% ");
  display.setCursor(0, 40);
  display.print("AQ: ");
  display.print(getAQLevel(air_quality));
  display.display();
}
