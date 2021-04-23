/**
 * WiFi Heatmap Scan n Step
 * 
 * This firmware is used to get a wifi heatmap, by averaging all wifi rssi at a certain
 * point, which is called a heatmap pixel. Then the servo will move vertically a certain
 * amount of degree to get all vertical pixels. After every vertical scan, the stepper
 * motor will rotate horizontally to continue scanning the next vertical area.
 * 
 * The movement will be a zigzag: top to bottom, step to side, bottom to up, step to side, ...
 * 
 * The total heatmap will be saved within the heatmap_pixel_map[] float array
 * 
 */

#include <Stepper.h>
#include <Servo.h>
#include <WiFi.h>

//=====================================
// STEPPER
// Controls stepper, giving horizontal rotations

#define STEPS_A_REVOLUTION 200  // change this to fit the number of steps per revolution for your motor

// initialize the stepper library on pins 8 through 11:
#define STPV_STEP_PIN 5   // GPIO5 = D1
#define STPV_DIR_PIN 4    // GPIO4 = D2
#define STPH_STEP_PIN 0   // GPIO0 = D3
#define STPH_DIR_PIN 2    // GPIO2 = D4

#define STEP_DELAY_MS 500

int vrtStep = 2;
int hrzStep = 20;

int vrtStepPos = 0;
int hrzStepPos = 0;

void setup_stepper() {
  // setup stepper vertical
  pinMode(STPV_STEP_PIN, OUTPUT);
  pinMode(STPV_DIR_PIN, OUTPUT);

  // setup stepper horizontal
  pinMode(STPH_STEP_PIN, OUTPUT);
  pinMode(STPH_DIR_PIN, OUTPUT);
}

void step_dir_cw(int pin, bool cw) {
  if (cw) {
    digitalWrite(pin,HIGH);
  } else {
    digitalWrite(pin,LOW);
  }
}

void step(int pin, int steps) {
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < steps; x++) {
    digitalWrite(pin,HIGH); 
    delayMicroseconds(STEP_DELAY_MS); 
    digitalWrite(pin,LOW); 
    delayMicroseconds(STEP_DELAY_MS); 
  }
}

void vertical_step(int steps) {
  // move vertically for strafing
  step(STPV_STEP_PIN, steps);
  vrtStepPos += steps;
}

void horizontal_step(int steps) {
  // move horizontally  for panning
  step(STPH_STEP_PIN, steps);
  hrzStepPos += steps;
}

//=====================================
// SERVO ---> NOT USED ANYMORE
// Controls servo, giving vertical rotations

//#define SERVO_PIN 4           // Declare the Servo pin  2 = D4
//#define SERVO_ANGLE_INC 10    // how much degree is added on one servo move
//#define SERVO_ANGLE_MAX 90
//
//Servo my_servo; // Create a servo object 
//bool turn_on = false;
//uint16_t servo_angle = 0;
//bool to_increase = true;
//
//void setup_servo() { 
//   my_servo.attach(SERVO_PIN); 
//}
//
//bool servo_move() { 
//  // the servo moves in a zigzag manner, it will move up to down, then down to up
//  my_servo.write(servo_angle);
//
//  int servo_angle_change = (to_increase?1:-1) *  SERVO_ANGLE_INC;
//  servo_angle = servo_angle + servo_angle_change; 
//  
//  if (servo_angle > SERVO_ANGLE_MAX) {
//    to_increase = false;
//    return true; // reached edge
//  } else if (servo_angle < 0) {
//    to_increase = true;
//    return true; // reached edge
//  } else {
//    return false;
//  }
//}

//=======================================
// WIFI HEATMAP
// Controls wifi mean rssi capture, then saving to heatmap_pixel_map

//#define SCAN_RESULT_PRINT
const int heatmap_pixel_pos_max = 1000;
float heatmap_pixel_map[heatmap_pixel_pos_max];
int heatmap_pixel_pos = 0;

bool add_heatmap_pixel_map(float _heatmap_pixel) {
  if (heatmap_pixel_pos < heatmap_pixel_pos_max) {
    heatmap_pixel_map[heatmap_pixel_pos] = _heatmap_pixel;
    heatmap_pixel_pos++;
    return true;
  } else {
    // fail to add because map already maxed
    return false;
  }
}

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

float wifi_meanrssi()
{
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  #ifdef SCAN_RESULT_PRINT
    Serial.println("scan done");
  #endif
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    #ifdef SCAN_RESULT_PRINT
      Serial.print(n);
      Serial.println(" networks found");
    #endif
    long totalRssi = 0;
    
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      #ifdef SCAN_RESULT_PRINT
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
        delay(10);
      #endif

      totalRssi += WiFi.RSSI(i);
    }
    
    float meanRssi = totalRssi/n;
    return meanRssi;
  }
}



//=====================================
// MAIN

void setup() {
  Serial.begin(115200);
  Serial.println("WiFi Heatmap Monster");

  setup_stepper();
  setup_wifi();
  
  Serial.println("setup: done");
}

void loop() {
  while(hrzStepPos <= STEPS_A_REVOLUTION) {
    // get heatmap_pixel
    int heatmap_pixel = wifi_meanrssi();
    bool success_add = add_heatmap_pixel_map(heatmap_pixel);
    if (!success_add) {
      Serial.println("heatmap_pixel_pos_max reached");
      break;
    }
    
    Serial.print("wifi mean rssi: ");
    Serial.println(heatmap_pixel);
    
    // continue vert_stepper zigzag movement, if reach edge return true
    vertical_step(vrtStep);

    // check if reached bottom or top, then change neg/pos accordingly
    bool isVrtEdgeBottom = vrtStepPos >= 50;
    bool isVrtEdgeTop = vrtStepPos <= 0;
    bool isVrtEdge = isVrtEdgeBottom || isVrtEdgeTop; // edge is position 90 degree or 0 degree
    delay(500);

    if (isVrtEdge) {
      // if reached edge, rotate horizontally
      horizontal_step(hrzStep);

      if (isVrtEdgeBottom) {
        vrtStep = -1 * abs(vrtStep);
      } else if (isVrtEdgeTop) {
        vrtStep = 1 * abs(vrtStep);
      }
      delay(500);
    }
  }

  Serial.println("Heatmap process done.");
  while(1);
}
