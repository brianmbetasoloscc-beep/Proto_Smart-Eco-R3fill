/*
 * SMART ECO-R3FILL - FIXED VERSION
 * Load Cell + HX711 + LCD I2C + Gizduino
 * 
 * Hardware Connections:
 * HX711:
 *   - DT (DOUT) → A0
 *   - SCK      → A1
 *   - VCC      → 5V
 *   - GND      → GND
 * 
 * Load Cell to HX711:
 *   - Red   → E+
 *   - Black → E-
 *   - White → A-
 *   - Green → A+
 * 
 * LCD I2C:
 *   - SDA → A4
 *   - SCL → A5
 *   - VCC → 5V
 *   - GND → GND
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// ========== PIN DEFINITIONS ==========
#define DOUT_PIN  A0  // HX711 DT pin
#define SCK_PIN   A1  // HX711 SCK pin

// ========== CONFIGURATION ==========
const float KNOWN_WEIGHT = 100.0;        // Calibration weight in grams
const float WEIGHT_THRESHOLD = 0.5;      // Minimum detectable weight (grams)
const float MAX_WEIGHT_ALLOWED = 3000.0; // Maximum weight: 3kg = 3000g

// ========== OBJECTS ==========
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Try 0x3F if 0x27 doesn't work

// ========== VARIABLES ==========
float calibrationFactor = -7050.0; // Will be calculated during calibration
float weight = 0.0;
float previousWeight = 0.0;
int waterVolume = 0;
bool isCalibrated = false;

// ========== FUNCTION DECLARATIONS ==========
void displayWelcome();
void displayInsertItem();
void displayWeighing();
void displayConversion(float grams, int milliliters);
void displayReject();
void calibrateScale();
void testLoadCell();

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);
  Serial.println("=================================");
  Serial.println("SMART ECO-R3FILL - Starting...");
  Serial.println("=================================");
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Display welcome message
  displayWelcome();
  delay(3000);
  
  // Initialize HX711
  Serial.println("Initializing HX711...");
  scale.begin(DOUT_PIN, SCK_PIN);
  
  // Check if HX711 is connected
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Checking sensor");
  
  if (!scale.is_ready()) {
    Serial.println("ERROR: HX711 not found!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR: HX711");
    lcd.setCursor(0, 1);
    lcd.print("Not Connected!");
    while (1); // Stop here - hardware problem
  }
  
  Serial.println("HX711 found!");
  delay(1000);
  
  // Perform automatic calibration
  calibrateScale();
  
  // Final tare after calibration
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Final zeroing...");
  scale.tare();
  delay(1000);
  
  // Ready to use
  Serial.println("System ready!");
  displayInsertItem();
}

// ========== MAIN LOOP ==========
void loop() {
  // Check if scale is ready
  if (!scale.is_ready()) {
    Serial.println("WARNING: Scale not ready!");
    delay(100);
    return;
  }
  
  // Read weight with averaging
  weight = scale.get_units(10);
  
  // Filter noise
  if (weight < WEIGHT_THRESHOLD) {
    weight = 0.0;
  }
  
  // Debug output
  Serial.print("Weight: ");
  Serial.print(weight, 2);
  Serial.println(" g");
  
  // Detect new item (significant weight increase)
  if (weight > previousWeight + WEIGHT_THRESHOLD) {
    Serial.println("Item detected!");
    
    // Show weighing message
    displayWeighing();
    delay(2000); // Let weight stabilize
    
    // Get stabilized weight
    weight = scale.get_units(20); // More readings for accuracy
    
    Serial.print("Final weight: ");
    Serial.print(weight, 2);
    Serial.println(" g");
    
    // Check if weight exceeds maximum
    if (weight > MAX_WEIGHT_ALLOWED) {
      Serial.println("REJECTED: Over max weight!");
      displayReject();
      delay(5000);
      scale.tare();
      previousWeight = 0.0;
      displayInsertItem();
    }
    // Accept if weight is valid
    else if (weight >= WEIGHT_THRESHOLD) {
      Serial.println("ACCEPTED!");
      
      // Convert to water volume (1g = 1ml)
      waterVolume = round(weight);
      
      Serial.print("Water to dispense: ");
      Serial.print(waterVolume);
      Serial.println(" ml");
      
      // Display conversion
      displayConversion(weight, waterVolume);
      delay(5000);
      
      // TODO: Add water dispensing code here
      // dispenseWater(waterVolume);
      
      // Reset for next item
      Serial.println("Resetting scale...");
      scale.tare();
      previousWeight = 0.0;
      displayInsertItem();
    }
  }
  
  previousWeight = weight;
  delay(100); // Small delay to prevent flooding serial monitor
}

// ========== DISPLAY FUNCTIONS ==========

void displayWelcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART ECO-R3FILL");
  lcd.setCursor(0, 1);
  lcd.print("   Welcome!     ");
  
  Serial.println("Display: Welcome screen");
}

void displayInsertItem() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INSERT ITEM");
  lcd.setCursor(0, 1);
  lcd.print("Max 3kg (1g=1ml)");
  
  Serial.println("Display: Insert item prompt");
}

void displayWeighing() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weighing...");
  lcd.setCursor(0, 1);
  lcd.print("Please wait");
  
  Serial.println("Display: Weighing...");
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
  
  Serial.println("Display: Conversion result");
}

void displayReject() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ITEM REJECTED!");
  lcd.setCursor(0, 1);
  lcd.print("Max weight: 3kg");
  
  Serial.println("Display: Item rejected");
}

// ========== CALIBRATION FUNCTION ==========

void calibrateScale() {
  Serial.println("=================================");
  Serial.println("Starting calibration...");
  Serial.println("=================================");
  
  // Step 1: Remove all weight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CALIBRATION");
  lcd.setCursor(0, 1);
  lcd.print("Remove weight");
  
  Serial.println("Step 1: Remove all weight");
  Serial.println("Waiting 5 seconds...");
  
  for (int i = 5; i > 0; i--) {
    lcd.setCursor(14, 1);
    lcd.print(i);
    delay(1000);
  }
  
  // Tare (set zero point)
  Serial.println("Setting zero point...");
  scale.set_scale();
  scale.tare();
  
  long offset = scale.read_average(20);
  Serial.print("Tare offset: ");
  Serial.println(offset);
  
  // Step 2: Place known weight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place known");
  lcd.setCursor(0, 1);
  lcd.print("weight (");
  lcd.print(KNOWN_WEIGHT, 0);
  lcd.print("g)");
  
  Serial.print("Step 2: Place ");
  Serial.print(KNOWN_WEIGHT, 0);
  Serial.println("g weight");
  Serial.println("Waiting 5 seconds...");
  
  for (int i = 5; i > 0; i--) {
    lcd.setCursor(14, 1);
    lcd.print(i);
    delay(1000);
  }
  
  // Read with known weight
  long rawReading = scale.read_average(20);
  Serial.print("Raw reading with weight: ");
  Serial.println(rawReading);
  
  // Calculate calibration factor
  calibrationFactor = (float)(rawReading - offset) / KNOWN_WEIGHT;
  
  Serial.print("Calculated calibration factor: ");
  Serial.println(calibrationFactor, 2);
  
  // Set the scale
  scale.set_scale(calibrationFactor);
  
  // Display calibration factor
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cal Factor:");
  lcd.setCursor(0, 1);
  lcd.print(calibrationFactor, 0);
  
  Serial.println("Calibration complete!");
  Serial.println("=================================");
  
  delay(3000);
  
  isCalibrated = true;
}

// ========== TEST FUNCTION (Optional - for debugging) ==========

void testLoadCell() {
  Serial.println("=================================");
  Serial.println("RAW LOAD CELL TEST");
  Serial.println("=================================");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Testing HX711");
  
  for (int i = 0; i < 10; i++) {
    if (scale.is_ready()) {
      long raw = scale.read();
      Serial.print("Raw reading: ");
      Serial.println(raw);
      
      lcd.setCursor(0, 1);
      lcd.print("Raw: ");
      lcd.print(raw);
    } else {
      Serial.println("HX711 NOT READY!");
      lcd.setCursor(0, 1);
      lcd.print("NOT READY!");
    }
    delay(500);
  }
  
  Serial.println("Test complete!");
  Serial.println("=================================");
}
