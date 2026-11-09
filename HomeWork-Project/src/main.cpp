#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1
);

// Pin definitions
#define DHT_PIN 4
#define DHT_TYPE DHT22

#define POT_PIN 34
#define PWM_LED 18
#define DAC_LED 25
#define BUZZER 19
#define PWM_LED_CHANNEL 0
#define BUZZER_CHANNEL 1

DHT dht(DHT_PIN, DHT_TYPE);

unsigned long previousTime = 0;

void setup() {

  Serial.begin(115200);

  // Start OLED
  Wire.begin(21, 22);

  if (!display.begin(
    SSD1306_SWITCHCAPVCC, 0x3C
  )) {
    Serial.println("OLED Error");
    while (true);
  }

  // Start DHT sensor
  dht.begin();

  // ADC configuration
  analogReadResolution(12);
  analogSetPinAttenuation(
    POT_PIN, ADC_11db
  );

  // PWM configuration
  ledcSetup(PWM_LED_CHANNEL, 5000, 8);
  ledcAttachPin(PWM_LED, PWM_LED_CHANNEL);

  // Buzzer PWM
  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER, BUZZER_CHANNEL);

  pinMode(DAC_LED, OUTPUT);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  Serial.println("Smart Room System Started");
}

void loop() {

  // Read ADC
  int adcValue = analogRead(POT_PIN);

  // Convert ADC into temperature limit
  int tempLimit = map(
    adcValue, 0, 4095, 25, 40
  );

  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT Sensor Error");
    delay(1000);
    return;
  }

  // PWM: Control LED brightness
  int brightness = map(
    constrain((int)(temperature * 10), 200, 400),
    200, 400, 0, 255
  );

  ledcWrite(PWM_LED_CHANNEL, brightness);

  // DAC: Analog output
  int dacValue = map(
    constrain((int)(temperature * 10), 0, 400),
    0, 400, 0, 255
  );

  dacWrite(DAC_LED, dacValue);

  // Temperature alarm
  bool alarm = temperature >= tempLimit;

  if (alarm) {
    ledcWrite(BUZZER_CHANNEL, 128);
  } else {
    ledcWrite(BUZZER_CHANNEL, 0);
  }

  // OLED display
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SMART ROOM MONITOR");

  display.setCursor(0, 13);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 24);
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.println("%");

  display.setCursor(0, 35);
  display.print("ADC: ");
  display.println(adcValue);

  display.setCursor(0, 46);
  display.print("Limit: ");
  display.print(tempLimit);
  display.println(" C");

  display.setCursor(0, 56);

  if (alarm) {
    display.print("WARNING! ALARM ON");
  } else {
    display.print("STATUS: NORMAL");
  }

  display.display();

  // Serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C | ADC: ");
  Serial.print(adcValue);
  Serial.print(" | DAC: ");
  Serial.print(dacValue);
  Serial.print(" | PWM: ");
  Serial.println(brightness);

  delay(2000);
}