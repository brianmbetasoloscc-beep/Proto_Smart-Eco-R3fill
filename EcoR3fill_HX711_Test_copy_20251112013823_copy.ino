#include "HX711.h"


const int LOADCELL_DOUT_PIN = A0; 
const int LOADCELL_SCK_PIN = A1;  

HX711 scale;

void setup() {
  Serial.begin(9600); 
  Serial.println("HX711 Calibration Sketch");
  Serial.println("-------------------------");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  
  if (scale.is_ready()) {
    Serial.println("HX711 is ready!");
  } else {
    Serial.println("--- ERROR: HX711 not found. Check wiring. ---");
    while (true); 
  }

  
  Serial.println("\nRemove all weight from the scale and press 't' to tare.");
  Serial.println("Once tared, place a known weight on the scale.");
  Serial.println("Then, enter the known weight (in grams) and press 'c' to calculate factor.");
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 't') { 
      Serial.println("\nTaring... please wait.");
      scale.tare();
      Serial.println("Scale tared to 0.");
      Serial.println("Place a known weight on the scale and enter its value (e.g., '100g') and press 'c'.");
    } else if (command == 'c') { 
      
      String weightStr = Serial.readStringUntil('g'); 
      float known_weight = weightStr.toFloat();

      if (known_weight == 0) {
        Serial.println("Invalid weight. Enter a number followed by 'g' (e.g., 100g).");
        return;
      }

      
      long raw_reading = scale.get_value(10); 

      if (raw_reading != 0) {
        float new_calibration_factor = raw_reading / known_weight;
        Serial.print("\nKnown Weight: ");
        Serial.print(known_weight);
        Serial.println(" g");
        Serial.print("Raw Reading with weight: ");
        Serial.println(raw_reading);
        Serial.print("Calculated CALIBRATION_FACTOR: ");
        Serial.println(new_calibration_factor, 2); 
        Serial.println("-------------------------");
        Serial.println("RECORD THIS FACTOR for your main code!");
        Serial.println("Remove weight, press 't' to re-tare and test again.");
      } else {
        Serial.println("No stable reading with weight. Try again.");
      }
    }
  }

  
  if (scale.is_ready()) {
    Serial.print("Current Raw Reading: ");
    Serial.println(scale.read());
  }
  delay(100);
}