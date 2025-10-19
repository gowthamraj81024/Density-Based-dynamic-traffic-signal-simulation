// Define pins for traffic lights (Arduino Uno Compatible)
int Lane[4][3] = {
  {13, 12, 11}, // Lane 1: Red, Yellow, Green
  {10, 9, 8},  // Lane 2: Red, Yellow, Green
  {7, 6, 5},   // Lane 3: Red, Yellow, Green
  {4, 3, 2}    // Lane 4: Red, Yellow, Green
};

// Define IR sensor pins for each lane
const int irPins[4] = {A0, A1, A2, A3};

// Traffic density storage
int trafficDensity[4] = {0, 0, 0, 0};

// Timing variables
int baseGreenTime = 5000; // Base green time
int yellowTime = 2000; // Yellow light duration
int minGreenTime = 3000; // Minimum green time before switching
int reductionFactor = 2000; // Amount to reduce green time if higher density lane is detected

void setup() {
  Serial.begin(9600); // Initialize serial communication
  
  // Set up traffic light and sensor pins
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      pinMode(Lane[i][j], OUTPUT);
      digitalWrite(Lane[i][j], LOW);
    }
    pinMode(irPins[i], INPUT);
  }
  // Ensure all lanes start with red on
  for (int i = 0; i < 4; i++) {
    digitalWrite(Lane[i][0], HIGH);
  }
}

void loop() {
  while (true) {
    for (int lane = 0; lane < 4; lane++) {
      measureTrafficDensity();
      int greenTime = baseGreenTime;
      
      if (trafficDensity[lane] == 3) {
        greenTime += 3000; // Increase green time if high density
      }

      // Turn all lanes red initially
      for (int i = 0; i < 4; i++) {
        digitalWrite(Lane[i][0], HIGH);
        digitalWrite(Lane[i][1], LOW);
        digitalWrite(Lane[i][2], LOW);
      }
      
      // Set green for the current lane
      digitalWrite(Lane[lane][0], LOW); // Turn off red
      digitalWrite(Lane[lane][2], HIGH); // Turn on green
      
      unsigned long startTime = millis();
      while (millis() - startTime < greenTime) {
        measureTrafficDensity();
        int newPriorityLane = getMaxDensityLane();
        if (newPriorityLane != lane && trafficDensity[newPriorityLane] > trafficDensity[lane]) {
          greenTime = max(minGreenTime, greenTime - reductionFactor); // Reduce green time but do not switch immediately
        }
      }
      
      // Blinking transition effect before switching
      int nextLane = (lane + 1) % 4;
      for (int i = 0; i < 10; i++) {
        digitalWrite(Lane[lane][2], LOW); // Blink green light OFF
        digitalWrite(Lane[nextLane][0], LOW); // Blink next lane's red OFF
        delay(100);
        digitalWrite(Lane[lane][2], HIGH); // Blink green light ON
        digitalWrite(Lane[nextLane][0], HIGH); // Blink next lane's red ON
        delay(100);
      }
      digitalWrite(Lane[lane][2], LOW); // Turn off green light
      digitalWrite(Lane[nextLane][0], LOW); // Turn off next lane's red
      
      // Turn on yellow for both lanes
      digitalWrite(Lane[lane][1], HIGH); // Current lane yellow ON
      digitalWrite(Lane[nextLane][1], HIGH); // Next lane yellow ON
      delay(yellowTime);
      
      // Turn off yellow lights before switching
      digitalWrite(Lane[lane][1], LOW);
      digitalWrite(Lane[nextLane][1], LOW);
      digitalWrite(Lane[nextLane][0], HIGH); // Ensure next lane's red is ON after transition
    }
  }
}

void measureTrafficDensity() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(Lane[i][0]) == HIGH) { // Measure only when red light is ON
      int irValue = digitalRead(irPins[i]);
      trafficDensity[i] = irValue == LOW ? 3 : 1; // LOW means vehicle detected (high density)
      
      Serial.print("Lane "); Serial.print(i + 1);
      Serial.print(" IR Value: "); Serial.print(irValue);
      Serial.print(" , Density: "); Serial.println(trafficDensity[i]);
    }
  }
}

int getMaxDensityLane() {
  int maxLane = 0;
  for (int i = 1; i < 4; i++) {
    if (trafficDensity[i] > trafficDensity[maxLane]) {
      maxLane = i;
    }
  }
  return maxLane;
}
