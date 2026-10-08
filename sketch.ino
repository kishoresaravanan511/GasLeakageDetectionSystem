#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== PIN CONFIGURATION =====
#define GAS_PIN 34
#define GREEN_LED 18
#define YELLOW_LED 19
#define RED_LED 23
#define BUZZER 25

// ===== OLED CONFIGURATION =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ===== GAS THRESHOLDS =====
// Simulation-only ADC thresholds (0-4095)
const int SAFE_LIMIT = 1400;
const int DANGER_LIMIT = 2800;

// Rapid-rise early warning
const int RAPID_RISE_LIMIT = 600;

// ===== SYSTEM VARIABLES =====
int previousGasValue = -1;
bool oledReady = false;

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  analogReadResolution(12);

  // ESP32 I2C: SDA=21, SCL=22
  Wire.begin(21, 22);

  oledReady = display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

  if (!oledReady) {
    Serial.println("OLED initialization failed!");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(10, 15);
    display.println("GasGuard");
    display.setTextSize(1);
    display.setCursor(15, 42);
    display.println("System Starting");
    display.display();
  }

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);

  Serial.println("======================");
  Serial.println("GASGUARD SYSTEM READY");
  Serial.println("======================");

  delay(1500);
}

void loop() {

  // ===== READ SENSOR =====
  int gasValue = analogRead(GAS_PIN);

  // ===== RAPID RISE DETECTION =====
  int rise = 0;

  if (previousGasValue >= 0) {
    rise = gasValue - previousGasValue;
  }

  bool rapidRise = rise >= RAPID_RISE_LIMIT;
  previousGasValue = gasValue;

  // ===== CLASSIFY GAS LEVEL =====
  String status;

  if (gasValue >= DANGER_LIMIT) {
    status = "DANGER";
  }
  else if (gasValue >= SAFE_LIMIT || rapidRise) {
    status = "WARNING";
  }
  else {
    status = "SAFE";
  }

  // ===== RESET OUTPUTS =====
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);

  // ===== LOCAL ALERT SYSTEM =====
  if (status == "SAFE") {
    digitalWrite(GREEN_LED, HIGH);
  }
  else if (status == "WARNING") {
    digitalWrite(YELLOW_LED, HIGH);
  }
  else {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 2000);
  }

  // ===== SERIAL MONITOR =====
  Serial.println("----------------------");

  Serial.print("Gas ADC: ");
  Serial.println(gasValue);

  Serial.print("Gas Level: ");
  Serial.print(gasValue * 100.0 / 4095.0, 1);
  Serial.println("% of ADC range");

  Serial.print("Status: ");
  Serial.println(status);

  Serial.print("Rise: ");
  Serial.println(rise);

  if (rapidRise) {
    Serial.println("Rapid gas rise detected!");
  }

  // ===== OLED DISPLAY =====
  if (oledReady) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("GASGUARD MONITOR");

    display.drawLine(
      0, 12, 127, 12,
      SSD1306_WHITE
    );

    display.setCursor(0, 18);
    display.print("Gas ADC: ");
    display.println(gasValue);

    display.setCursor(0, 29);
    display.print("Level: ");
    display.print(gasValue * 100.0 / 4095.0, 1);
    display.println("%");

    display.setTextSize(2);
    display.setCursor(0, 42);
    display.println(status);

    if (rapidRise) {
      display.setTextSize(1);
      display.setCursor(95, 30);
      display.print("RISE");
    }

    display.display();
  }

  delay(500);
}