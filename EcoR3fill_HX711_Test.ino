
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// HX711 Pins
#define DOUT_PIN  A0  // DT pin
#define SCK_PIN   A1  // SCK pin

// Initialize HX711
HX711 scale;

// Initialize LCD (address 0x27 for 16x2, change if different)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Calibration factor - adjust this based on your load cell
float calibrationFactor = -7050; // Change this value after calibration

// Variables
float weight = 0;
float previousWeight = 0;
int waterVolume = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize HX711
  scale.begin(DOUT_PIN, SCK_PIN);
  
  // Display welcome message
  displayWelcome();
  calibrateScale();
  delay(2000);
  
  // Calibrate/tare the scale
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  
  scale.set_scale(calibrationFactor);
  scale.tare(); // Reset to zero
  
  delay(1000);
  
  // Show insert item prompt
  displayInsertItem();
}

void loop() {
  // Read weight from load cell
  weight = scale.get_units(10); // Average of 10 readings
  
  // Filter out negative values and noise (threshold: 0.5g)
  if (weight < 0.5) {
    weight = 0;
  }
  
  // Check if new item detected (weight increased)
  if (weight > previousWeight + 0.5) {
    // Item detected
    displayWeighing();
    delay(2000); // Stabilize reading
    
    // Get final weight
    weight = scale.get_units(10);
    
    if (weight >= 0.5) { // Minimum recyclable weight
      // Convert weight to water volume (1g = 1ml)
      waterVolume = round(weight);
      
      // Display conversion
      displayConversion(weight, waterVolume);
      delay(5000); // Show result for 5 seconds
      
      // Here you would trigger water dispensing
      // dispenseWater(waterVolume);
      
      // Reset and wait for next item
      scale.tare();
      displayInsertItem();
    }
  }
  
  previousWeight = weight;
  
  // Debug output
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" g");
  
  delay(100);
}

void displayWelcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART ECO-R3FILL");
  lcd.setCursor(0, 1);
  lcd.print("   Welcome!     ");
}

void displayInsertItem() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Insert Item");
  lcd.setCursor(0, 1);
  lcd.print("1g = 1ml");
  
  // Optional: Add loading animation for 10 seconds
  for (int i = 0; i < 10; i++) {
    lcd.setCursor(13 + (i % 3), 1);
    lcd.print(".");
    delay(1000);
    if (i % 3 == 2) {
      lcd.setCursor(13, 1);
      lcd.print("   ");
    }
  }
}

void displayWeighing() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weighing");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
}

void displayConversion(float grams, int milliliters) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weight: ");
  lcd.print(grams, 1);
  lcd.print("g");
  
  lcd.setCursor(0, 1);
  lcd.print("Water: ");
  lcd.print(milliliters);
  lcd.print("ml");
}

// Function to calibrate the load cell
void calibrateScale() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remove all");
  lcd.setCursor(0, 1);
  lcd.print("weight");
  delay(5000);
  
  scale.set_scale();
  scale.tare();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place known");
  lcd.setCursor(0, 1);
  lcd.print("weight (100g)");
  delay(5000);
  
  long reading = scale.get_units(10);
  calibrationFactor = reading / 100.0; // 100g reference weight
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cal Factor:");
  lcd.setCursor(0, 1);
  lcd.print(calibrationFactor);
  delay(3000);
}
