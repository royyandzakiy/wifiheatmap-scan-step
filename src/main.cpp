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
 * The total heatmap will be saved within the heatmapPixelMap[] float array
 * 
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Stepper.h>

//=====================================
// STEPPER
// Controls stepper, giving horizontal rotations

// change this to fit the number of steps per revolution for your motor
#define MAX_VERT_STEP 200
#define MAX_HORZ_STEP 200 

// Stepper Control Related Pins
#define STEP_VERT_PIN  26
#define DIR_VERT_PIN   16 
#define STEP_HORZ_PIN  17
#define DIR_HORZ_PIN   14
#define LIM_VERT_PIN   13
#define LIM_HORZ_PIN   23

#define STEP_DELAY_MS 500

const int numOfPixelX = 5;
const int numOfPixelY = 20;
const int heatmapResolution = numOfPixelX * numOfPixelY;

const int xStep = 5;
const int yStep = 5;

int xPos = 0;
int yPos = 0;

int xDirection;

Stepper vertStepper(MAX_VERT_STEP, STEP_VERT_PIN, DIR_VERT_PIN);
Stepper horzStepper(MAX_HORZ_STEP, STEP_HORZ_PIN, DIR_HORZ_PIN);

//=======================================
// WIFI HEATMAP
// Controls wifi mean rssi capture, then saving to heatmapPixelMap

// #define SCAN_RESULT_PRINT

float heatmapPixelMap[heatmapResolution];
int heatmapPixelPos = 0;

bool addHeatmapPixelMap(float _heatmap_pixel) {
  if (heatmapPixelPos > heatmapResolution) return false;

  heatmapPixelMap[heatmapPixelPos] = _heatmap_pixel;
  heatmapPixelPos++;
  return true;
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

void moveStepper() {
  horzStepper.step(xStep*xDirection);
  delay(STEP_DELAY_MS);
  xPos += xDirection;

  bool isHorzEdgeLeft = xPos >= numOfPixelX;
  bool isHorzEdgeRight = xPos <= 0;
  bool isHitEdge = isHorzEdgeLeft || isHorzEdgeRight;

  if (isHitEdge) {
    vertStepper.step(yStep);
    delay(STEP_DELAY_MS);
    yPos++;

    if (isHorzEdgeLeft) {
      xDirection = -1;
    } else {
      xDirection = 1;
    }
  }
}

//=====================================
// MAIN

void setup() {
  Serial.begin(115200);
  Serial.println("WiFi Heatmap Monster");

  setupWiFi();
  avgWiFiRssi();
  vertStepper.setSpeed(10);
  horzStepper.setSpeed(20);
  
  Serial.println("setup: done");
}

void loop() {
  int heatmapRead;

  do {
    // heatmapRead = avgWiFiRssi(); 
    heatmapRead = 0; // mechanical test
    moveStepper();

    Serial.print(xPos);
    Serial.print(",");
    Serial.print(yPos);
    Serial.print(",");
    Serial.print(heatmapRead);
    Serial.print("\n");
  } while (addHeatmapPixelMap(heatmapRead));

  Serial.println("Heatmap process done.");
  while(1);
}