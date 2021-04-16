 # WiFi Heatmap Scan n Step
 
This firmware is used to get a wifi heatmap, by averaging all wifi rssi at a certain point, which is called a heatmap pixel. Then the servo will move vertically a certain amount of degree to get all vertical pixels. After every vertical scan, the stepper motor will rotate horizontally to continue scanning the next vertical area.
 
The movement will be a zigzag: top to bottom, step to side, bottom to up, step to side, ...
 
The total heatmap will be saved within the heatmap_pixel_map[] float array
