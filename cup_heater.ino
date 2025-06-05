#include <NTC_Thermistor.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define TARGET_TEMP_C 40
#define DELTA_TEMP_C 2

/*****************************************************************************/
/*************************** FSR Sensor Variables ****************************/
/*****************************************************************************/

#define FSR_PIN 4

bool cupPresent = false;

/*****************************************************************************/
/*************************** OLED Display Variables **************************/
/*****************************************************************************/

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*****************************************************************************/
/************************** NTC Thermistor Variables *************************/
/*****************************************************************************/

#define SENSOR_PIN              34
#define REFERENCE_RESISTANCE    10000
#define NOMINAL_RESISTANCE      20000
#define NOMINAL_TEMPERATURE     25
#define B_VALUE                 3900
#define ESP32_ANALOG_RESOLUTION 4095
#define ESP32_ADC_VREF_MV       3300

Thermistor* thermistor = new NTC_Thermistor_ESP32(
  SENSOR_PIN,
  REFERENCE_RESISTANCE,
  NOMINAL_RESISTANCE,
  NOMINAL_TEMPERATURE,
  B_VALUE,
  ESP32_ADC_VREF_MV,
  ESP32_ANALOG_RESOLUTION
);

double currTemp = 0;

/*****************************************************************************/
/****************************** Relay Variables ******************************/
/*****************************************************************************/

#define RELAY_PIN 12

void setup() {
  Serial.begin(115200);

  pinMode(FSR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
}

void updateCupPresence()
{
  int force = analogRead(FSR_PIN);
  cupPresent = force > 0;
}

void displayInfo()
{
  display.clearDisplay();

  display.setCursor(0, 8);
  display.setTextColor(1);
  display.setTextSize(3);
  display.setTextWrap(true);
  display.cp437(true);

  display.print(currTemp);
  display.write(0xF8);
  display.println("C");

  display.drawFastHLine(0, 40, 150, 1);

  display.setCursor(16, 47);
  display.setTextSize(2);
  const int target = (cupPresent) ? TARGET_TEMP_C : 0;
  display.print("Tgt: ");
  display.print(target);
  display.write(0xF8);
  display.println("C");

  display.display();
}

void activateRelay()
{
  if (cupPresent) {
    if (currTemp <= TARGET_TEMP_C - (DELTA_TEMP_C / 2.0)) digitalWrite(RELAY_PIN, HIGH);
    if (currTemp >= TARGET_TEMP_C + (DELTA_TEMP_C / 2.0)) digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
}

void currentTemp()
{
  currTemp = thermistor->readCelsius();
}

void loop() {
  updateCupPresence();
  currentTemp();
  activateRelay();
  displayInfo();
  delay(500);
}
