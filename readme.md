 # WiFi Heatmap Scan n Step
 
This firmware is used to get a wifi heatmap, by averaging all wifi rssi at a certain point, which is called a heatmap pixel. Then the servo will move vertically a certain amount of degree to get all vertical pixels. After every vertical scan, the stepper motor will rotate horizontally to continue scanning the next vertical area.
 
The movement will be a zigzag: top to bottom, step to side, bottom to up, step to side, ...
 
The total heatmap will be saved within the heatmap_pixel_map[] float array

## Getting Started

- install Arduino IDE
- add board ESP32 if not yet
- add ServoESP32 library from libs folder. Sketch > Include Library > Add .zip Library
- open wifiheatmap-scan-step.ino
- connect ESP32 board
- choose correct board. Tools > Board > "DOIT ESP32 DEVKIT V1"
- choose correct port. Tools > Port > [choose correct port]
- upload
