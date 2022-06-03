---
layout: default
title: Code for the Sensors
nav_order: 4
---

# Sensors

<div style="text-align: justify"> Sensors are essential to the design of our boat to analyse the wind direction, the speed of the boat and it's location. Here are the tools we used : 

## GPS 
  
<div style="text-align: justify"> A GPS allows us to measure time, location and speed. We wanted to use it to calculate the average speed of our boat to optimise the angle of our sail. Furthermore, we wanted to use the location to allow our boat to follow a trajectory independently. Unfortunately, we were not able to get our hands on a working GPS so could not implement these features with it. But we do recommend using one for the reasons above ! If you have the opportunity to get one, we recommend using the TinyGPS++ library from Arduiniana which is very easy to use and beginner friendly. </div>
<br/>
As an alternative solution, we have decided to use the IMU’s accelerometer and gyroscope to get the same, albeit less precise, information. 

## IMU
  
 <div style="text-align: justify"> First of all, what is an IMU and why do we need it ? An IMU is a module composed of an accelerometer and a gyroscope. Ours, also houses a temperature sensor but it can also have a magnetometer in some cases. To connect it to our Arduino, we can just use the analog pins A4, A5 for the SDA and SCL pins, the 5V pin for the VCC input and the ground pin as showed in the figure below. </div>
  
   ![](assets/IMU.png)
  
<br/>
 <div style="text-align: justify"> Be careful, not all IMUs can be directly powered from a 5V source, some need an additional level shifter from 5V to 3.3V. If you have an IMU 6050 GY-521 like us for example, you should be just fine with a 5V output. </div>
  
 <br/>
  
 To code our IMU, we used the MPU6050 library created by jarzebski which allowed to create the functions detailed below : 
    
  <br/>
  
  ### test_speed() 
  
  This method integrates the data of the accelerometer along the X-axis to find the linear speed of our boat uses the formula : `speed = acceleration * time + speed` to update the speed each time it is called
  
  ### location_update()
  
  ### update_arrival()
  
  ### arrival()
  `arrival()` calls `location_update()` and checks whether the boat has arrived to the next destination in the array of targets
  
  ### get_time()
  
  `get_time` works with two global variables previous and present and allows to calculate the time passed (in seconds) since the last time it was called. It uses the `millis()` function of the arduino which returns time in milliseconds. 
  
  ### create_target()
  
  `create_target` takes as input two coordinates x and y to translates them to an angle and radius to return a Location target for the boat. This function is useful in the setup to initialise our array of stops for the boat to go to
  

## Wind vane
