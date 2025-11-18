#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#inclu#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// HX711 Pins
#define DOUT_PIN  A0  // DT pin (Data)
#define SCK_PIN   A1  // SCK pin (Clock)

// Known Weight for Calibration (e.g., 100 grams)
const float KNOWN_WEIGHT = 100.0; 

// Initialize HX711
HX711 scale;

// Initialize LCD (address 0x27 for 16x2, change if different)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Calibration factor - will be calculated during setup
float calibrationFactor = -7050.0; // Initial value (will be overwritten)

// Variables
float weight = 0.0;
float previousWeight = 0.0;
int waterVolume = 0;
// Define the maximum weight allowed (3kg = 3000g)
const float MAX_WEIGHT_ALLOWED = 3000.0; 


// --- Function Prototypes (Declarations) ---
void displayWelcome();
void displayEnsureEmpty(); // NEW/MODIFIED
void displayInsertItem();
void displayWeighing();
void displayConversion(float grams, int milliliters);
void displayReject();
void calibrateScale();


void setup() {
    Serial.begin(9600);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    
    // Initialize HX711
    scale.begin(DOUT_PIN, SCK_PIN);
    
    // 1. Display welcome message for 5 seconds
    displayWelcome();
    delay(5000); 

    // 2. Ensure scale is empty and Tare (Zero) the scale
    displayEnsureEmpty();
    
    // 3. Perform calibration and set the correct scale factor
    // NOTE: Calibration requires user action (placing a known weight)
    calibrateScale(); 
    
    // Final zeroing after setting the correct scale factor
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scale Ready!");
    scale.tare(); 
    delay(1000);
    
    // 4. Show the initial "Insert Item" prompt
    displayInsertItem(); 
}

void loop() {
    // Read weight from load cell
    weight = scale.get_units(10); 
    
    if (weight < 0.5) {
        weight = 0.0;
    }
    
    // Check if new item detected (weight increased significantly)
    if (weight > previousWeight + 0.5) {
        
        // 5. Display "please wait...."
        displayWeighing();
        delay(2000); 
        
        weight = scale.get_units(20); 
        
        // Check if ACCEPT or REJECT
        if (weight > MAX_WEIGHT_ALLOWED) {
            displayReject(); 
        } 
        else if (weight >= 0.5) { 
            // Item accepted
            
            // Weight and Convert to ml
            waterVolume = round(weight); 
            
            // 6. Display conversion
            displayConversion(weight, waterVolume);
            delay(5000); 
            
            // Reset and wait for next item
            scale.tare();
            previousWeight = 0.0; 
            displayInsertItem();
        }
    }
    
    previousWeight = weight;
    
    // Debug output
    Serial.print("Weight: ");
    Serial.print(weight, 2); 
    Serial.println(" g");
    
    delay(100);
}


// --- DISPLAY FUNCTIONS ---

// 1. Smart Eco-R3fill.... (5 seconds)
void displayWelcome() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SMART ECO-R3FILL");
    lcd.setCursor(0, 1);
    lcd.print("   Welcome!     ");
}

// 2. Ensure that the scale is empty (for taring/zeroing)
void displayEnsureEmpty() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ensure scale");
    lcd.setCursor(0, 1);
    lcd.print("is EMPTY (Tare)");
    scale.tare(); // Perform the tare here
    delay(2000);
}

// 3. Insert Item
void displayInsertItem() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INSERT ITEM"); 
    lcd.setCursor(0, 1);
    lcd.print("Max 3Kg (1g=1ml)"); 
}

// 4. Please wait....
void displayWeighing() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WEIGHING");
    lcd.setCursor(0, 1);
    lcd.print("Please wait....");
}

// 5. Conversion (100g=1ml) - Adjusted to show actual result
void displayConversion(float grams, int milliliters) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight: ");
    lcd.print(grams, 1);
    lcd.print("g");
    
    lcd.setCursor(0, 1);
    // Display the conversion result in the requested format
    lcd.print(milliliters);
    lcd.print("ml (1g=1ml)");
}

// Function to handle rejection (Optional: you can remove this if not needed)
void displayReject() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ITEM REJECTED!");
    lcd.setCursor(0, 1);
    lcd.print("Max weight is 3Kg");
    delay(5000); 
    scale.tare();
    previousWeight = 0.0;
    displayInsertItem();
}

// --- CALIBRATION FUNCTION (Critical Logic) ---
void calibrateScale() {
    // 1. Prompt user to remove all weight for the RAW offset reading
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CALIBRATION MODE");
    lcd.setCursor(0, 1);
    lcd.print("Remove All Weight");
    delay(5000);
    
    long offset = scale.read_average(20);
    
    // 2. Prompt user to place the known weight
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place known");
    lcd.setCursor(0, 1);
    lcd.print("weight (");
    lcd.print(KNOWN_WEIGHT, 0);
    lcd.print("g)");
    delay(5000);
    
    long rawReading = scale.read_average(20);
    
    // 3. Calculate the true calibration factor
    calibrationFactor = (float)(rawReading - offset) / KNOWN_WEIGHT;
    
    // 4. Set the calculated factor
    scale.set_scale(calibrationFactor); 
    
    // 5. Display the calculated factor
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cal Factor Set:");
    lcd.setCursor(0, 1);
    lcd.print(calibrationFactor, 0);
    delay(3000);
}de <HX711.h>

// HX711 Pins
#define DOUT_PIN  A0  // DT pin (Data)
#define SCK_PIN   A1  // SCK pin (Clock)

// Known Weight for Calibration (e.g., 100 grams)
const float KNOWN_WEIGHT = 100.0; 

// Initialize HX711
HX711 scale;

// Initialize LCD (address 0x27 for 16x2, change if different)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Calibration factor - will be calculated during setup
float calibrationFactor = -7050.0; // Initial value (will be overwritten)

// Variables
float weight = 0.0;
float previousWeight = 0.0;
int waterVolume = 0;
// Define the maximum weight allowed (3kg = 3000g)
const float MAX_WEIGHT_ALLOWED = 3000.0; 


// --- Function Prototypes (Declarations) ---
void displayWelcome();
void displayInsertItem();
void displayWeighing();
void displayConversion(float grams, int milliliters);
void displayReject(); // NEW function for rejection
void calibrateScale();

// --- SETUP ---
void setup() {
    Serial.begin(9600);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    
    // Initialize HX711
    scale.begin(DOUT_PIN, SCK_PIN);
    
    // 1. Display welcome message for 5 seconds
    displayWelcome();
    delay(5000); 

    // 2. Perform calibration (This includes zeroing)
    calibrateScale();
    
    // 3. Final zeroing after setting the correct scale factor
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scale Ready!");
    scale.tare(); 
    delay(1000);
    
    // 4. Show the initial "Insert Item" prompt
    displayInsertItem(); 
}

// --- LOOP ---
void loop() {
    // Read weight from load cell
    weight = scale.get_units(10); 
    
    if (weight < 0.5) {
        weight = 0.0;
    }
    
    // Check if new item detected (weight increased significantly)
    if (weight > previousWeight + 0.5) {
        
        displayWeighing();
        delay(2000); 
        
        weight = scale.get_units(20); 
        
        // --- 3. Detect if it is ACCEPT or REJECT ---
        if (weight > MAX_WEIGHT_ALLOWED) {
            displayReject(); // Reject if over max weight (3kg)
        } 
        else if (weight >= 0.5) { 
            // Item accepted
            
            // 4. Weight and Convert to ml
            waterVolume = round(weight); 
            
            displayConversion(weight, waterVolume);
            delay(5000); 
            
            // Reset and wait for next item
            scale.tare();
            previousWeight = 0.0; 
            displayInsertItem();
        }
    }
    
    previousWeight = weight;
    
    // Debug output
    Serial.print("Weight: ");
    Serial.print(weight, 2); 
    Serial.println(" g");
    
    delay(100);
}


// --- DISPLAY FUNCTIONS ---

// 1. Smart Eco-R3fill.... (5 seconds)
void displayWelcome() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SMART ECO-R3FILL");
    lcd.setCursor(0, 1);
    lcd.print("   Welcome!     ");
}

// 2. Insert Item (New logic: Removed the long delay loop)
void displayInsertItem() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INSERT ITEM"); // Changed to ALL CAPS for clarity
    lcd.setCursor(0, 1);
    lcd.print("Max 3Kg (1g=1ml)"); 
}

// Function to handle rejection
void displayReject() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ITEM REJECTED!");
    lcd.setCursor(0, 1);
    lcd.print("Max weight is 3Kg");
    delay(5000); 
    scale.tare();
    previousWeight = 0.0;
    displayInsertItem();
}

void displayWeighing() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weighing");
    lcd.setCursor(0, 1);
    lcd.print("Please wait...");
}

// 4. Weight conversion (ex: 1grams = 1ml)
void displayConversion(float grams, int milliliters) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight: ");
    lcd.print(grams, 1);
    lcd.print("g");
    
    lcd.setCursor(0, 1);
    lcd.print("Water: ");
    lcd.print(milliliters);
    lcd.print("ml (1g=1ml)");
}

// --- CALIBRATION FUNCTION (Critical, no changes from previous fix) ---
void calibrateScale() {
    // ... (The corrected calibration logic remains here to ensure accuracy) ...
    // It prompts to remove weight, place known weight (100g), calculates factor, and sets scale.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Remove all");
    lcd.setCursor(0, 1);
    lcd.print("weight (5s)");
    delay(5000);
    
    long offset = scale.read_average(20);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place known");
    lcd.setCursor(0, 1);
    lcd.print("weight (");
    lcd.print(KNOWN_WEIGHT, 0);
    lcd.print("g)");
    delay(5000);
    
    long rawReading = scale.read_average(20);
    
    calibrationFactor = (float)(rawReading - offset) / KNOWN_WEIGHT;
    
    scale.set_scale(calibrationFactor); 
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cal Factor:");
    lcd.setCursor(0, 1);
    lcd.print(calibrationFactor, 0);
    delay(3000);
}
