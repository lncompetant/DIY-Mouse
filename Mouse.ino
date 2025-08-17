/**
 * This example turns the ESP32 into a Bluetooth LE mouse that continuously moves the mouse.
 */

// Include the necessary libraries
#include <BleMouse.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Placeholder pin values for mouse buttons (these aren't used in this example but are kept for context)
const int buttons[] = {1, 2, 3}; 


// Create instances of the libraries
Adafruit_MPU6050 mpu;
BleMouse bleMouse;

// Variables to store sensor data
sensors_event_t a, g, temp;

//variables for debouncing
bool lastButtonState = HIGH; // Assume pull-up resistor, button not pressed initially
unsigned long lastDebounceTime = 0; // The last time the output pin was toggled
const unsigned long debounceDelay = 50; // The debounce time; increase if you see bounces
    
bool debounce(int pin) {
  // Read the current state of the button
  bool reading = digitalRead(pin);

  // If the button state has changed (from lastButtonState)
  if (reading != lastButtonState) {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }

  // If enough time has passed since the last state change (debounceDelay)
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // And the current reading is different from the stable state
    // (this check is crucial to ensure it's not still bouncing after the delay)
    if (reading != lastButtonState) {
      // Then the button state is truly stable, so update lastButtonState
      lastButtonState = reading;
    }
  }
  // Return the currently stable button state
  return lastButtonState;
}

void setup() {
  // Start the serial communication for debugging
  Serial.begin(115200);

  // Small delay to allow serial to connect
  delay(10); 
  Serial.println("Starting BLE and Accelerometer!");
  
  // Start the Bluetooth LE mouse service
  bleMouse.begin();

  // Initialize the MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("MPU6050 initialization failed! Check wiring or I2C address.");
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 Found!");

  // --- MPU6050 Sensor Configuration ---
  
  // Set the Accelerometer range to +/-2G
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }

  // Set the Gyroscope Range to +/-500 deg/s
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  // Set the filter bandwidth to 5 Hz
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }
  
  Serial.println("Setup complete!");
}

void loop() {
  // Check if the Bluetooth mouse is connected to a host device
  if (bleMouse.isConnected()) {
    
    
    // Read the Accelerometer data
    mpu.getEvent(&accel, &gyro, &temp);

    // Get the accelerometer values for X and Y axes
    float accelX = accel.acceleration.x;
    float accelY = accel.acceleration.y;

    // Use a threshold to prevent the mouse from moving when the device is still
    if (abs(accelX) > 0.1 || abs(accelY) > 0.1) {
      // Scale the accelerometer data to a reasonable mouse movement value
      // This is a simple linear scaling. You might need to adjust the values.
      int moveX = (int)(-accelY * 5);
      int moveY = (int)(accelX * 5);
      
      // Move the mouse cursor
      bleMouse.move(moveX, moveY);
    }

    //  Loop through all buttons and see which needs to be clicked  NEEDS TO BE CHECKED
    int numButtons = sizeof(buttons) / sizeof(buttons[0]);
    for(int i = 0; i < numButtons; i++){
      // Call debounce for each button, passing its index
      bool buttonIsPressed = !debounce(buttons[i], i); // Invert logic if using INPUT_PULLUP (LOW when pressed)
      
      // Example: If left button (buttons[0]) is pressed, send a click
      if (i == 0 && buttonIsPressed) { // Assuming buttons[0] is the left mouse button
        bleMouse.press(MOUSE_LEFT);
      } else if (i == 0 && !buttonIsPressed) {
        bleMouse.release(MOUSE_LEFT);
      }
      // You'd add similar logic for middle (buttons[1]) and right (buttons[2]) buttons here
      // if (i == 1 && buttonIsPressed) { bleMouse.press(MOUSE_MIDDLE); } else if (i == 1 && !buttonIsPressed) { bleMouse.release(MOUSE_MIDDLE); }
      // if (i == 2 && buttonIsPressed) { bleMouse.press(MOUSE_RIGHT); } else if (i == 2 && !buttonIsPressed) { bleMouse.release(MOUSE_RIGHT); }
    }    



   // Small delay to prevent flooding the Bluetooth connection and to save power
   delay(10);
  }
}