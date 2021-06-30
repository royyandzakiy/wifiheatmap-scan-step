#include <Arduino.h>

/**
 * WiFi Heatmap Scan n Step
 * 
 * This firmware is used to get a wifi heatmap, by averaging all wifi rssi at a certain
 * point, which is called a heatmap pixel. Then the motor will move vertically a certain
 * amount of degree to get all vertical pixels. After every vertical scan, the stepper
 * motor will rotate horizontally to continue scanning the next vertical area.
 * 
 * The movement will be a zigzag: top to bottom, step to side, bottom to up, step to side, ...
 * 
 * The total heatmap will be saved within the heatmap_pixel_map[] float array
 * 
 */

#include <WiFi.h>
#include <Stepper.h>

//=====================================
// STEPPER
// Controls stepper, giving horizontal rotations

#define STEPS_A_REVOLUTION 200  // change this to fit the number of steps per revolution for your motor

// Stepper Control Related Pin Mapping
#define STEP_X_PIN  26
#define DIR_X_PIN   16 
#define STEP_Z_PIN  17
#define DIR_Z_PIN   14
#define LIM_X_PIN   13
#define LIM_Z_PIN   23

#define STEP_DELAY_MS 500

int vrtStep = 5;
int hrzStep = 20;

int vrtStepPos = 0;
int hrzStepPos = 0;

int pixelCount = 0;

int verticalStepDirection;

Stepper vertStepper(STEPS_A_REVOLUTION, STEP_X_PIN, DIR_X_PIN);
Stepper horzStepper(STEPS_A_REVOLUTION, STEP_Z_PIN, DIR_Z_PIN);

//=======================================
// WIFI HEATMAP
// Controls wifi mean rssi capture, then saving to heatmap_pixel_map

// #define SCAN_RESULT_PRINT
const int heatmap_pixel_pos_max = 1000;
float heatmap_pixel_map[heatmap_pixel_pos_max];
int heatmap_pixel_pos = 0;

bool addHeatmapPixelMap(float _heatmap_pixel) {
  if (heatmap_pixel_pos < heatmap_pixel_pos_max) {
    heatmap_pixel_map[heatmap_pixel_pos] = _heatmap_pixel;
    heatmap_pixel_pos++;
    return true;
  } else {
    // fail to add because map already maxed
    return false;
  }
}

void setupWiFi() {
  Serial.print("Setting up WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  
  delay(100);
  Serial.println("done");
}

float avgWiFiRssi() {
  int n = WiFi.scanNetworks();
  long sumRssi = 0;

  #ifdef SCAN_RESULT_PRINT
  Serial.print("Scan done ");
  Serial.print(n);
  Serial.println(" networks found");
  #endif

  // Print SSID and RSSI for each network found
  int i = 0;

  while (i < n) {
    #ifdef SCAN_RESULT_PRINT
    Serial.print(i+1);
    Serial.print(": (");
    Serial.print(WiFi.RSSI(i));
    Serial.print(")");
    Serial.println(WiFi.SSID(i));
    #endif

    sumRssi += WiFi.RSSI(i);
    ++i;
  }
  
  return (n != 0) ? sumRssi / n : 0;
}



//=====================================
// MAIN

void setup() {
  Serial.begin(115200);
  Serial.println("WiFi Heatmap Monster");

  setupWiFi();
  avgWiFiRssi();
  vertStepper.setSpeed(60);
  horzStepper.setSpeed(60);
  
  Serial.println("setup: done");
}

void loop() {
  while (pixelCount <= heatmap_pixel_pos_max) {
    // get heatmap_pixel
    int heatmap_pixel = avgWiFiRssi();
    bool success_add = addHeatmapPixelMap(heatmap_pixel);
    if (!success_add) {
      Serial.println("heatmap_pixel_pos_max reached");
      break;
    }
    
    // continue vert_stepper zigzag movement, if reach edge return true
    vertStepper.step(vrtStep);
    delay(STEP_DELAY_MS);
    vrtStepPos += verticalStepDirection;

    // check if reached bottom or top, then change neg/pos accordingly
    bool isVrtEdgeBottom = vrtStepPos >= 20;
    bool isVrtEdgeTop = vrtStepPos <= 0;
    bool reachVerticalEdge = isVrtEdgeBottom || isVrtEdgeTop; // edge is position 90 degree or 0 degree

    if (reachVerticalEdge) {
      vrtStep = -1 * vrtStep;
      horzStepper.step(hrzStep);
      delay(STEP_DELAY_MS);
      hrzStepPos++;

      if (isVrtEdgeBottom) {
        verticalStepDirection = -1;
      }
      if (isVrtEdgeTop) {
        verticalStepDirection = 1;
      }
    }

    Serial.print(hrzStepPos);
    Serial.print(",");
    Serial.print(vrtStepPos);
    Serial.print(",");
    Serial.print(heatmap_pixel);
    Serial.print("\n");

    pixelCount++;
  }

  Serial.println("Heatmap process done.");
  while(1);
}