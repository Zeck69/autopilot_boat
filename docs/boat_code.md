---
layout: default
title: Principles of Automation for Sailboat
nav_order: 5
---

# Sailing principles



## Movement of the sail

<br/>

Degree of freedom for boat witha given wind             |  Degree of freedom of the sail
:-------------------------:|:-------------------------:
![](assets/wind.png)  |  ![](assets/sail.png)



<div style="text-align: justify"> Before diving in the code, we have to understand how to move the boat with a given wind. First, the boat can only be directed in a direction at least 45° appart from the wind. The closest the boat gets to the wind cone (in red in the first) the closest our sail must be of the opposite diagonal (the red diagonal in the second image if the boat is at 45° to the right to the wind). </div>
<br/>
<em>For the following code we will talk about degrees between -179 (to the left) and 180 (to the right) for the position of the boat with respect to the wind that is our reference at 0 (cf. <b> method degrees_limit(int value) </b>)</em>. <br/><br/>

## Basic movement

<em>First thing, we have to positionate the sail automatically with a given wind to move forward</em>

* Predict an initial sail poition (cf. `degree_pred...(int boat_degree)`)
    * We first try to obtain geometrically the optimal position of the sail, computing the impact of the wind in the sail. This method was no where close to the desired objective and was very imprecise with the most extreme values (such as 0° or 180°).
    * Our second approach was to use pennants to experimentally see the impact of the wind in the sail and do a regression with the data to obtain a precise function. (%4.3 standard deviation)
    <br/>

    ![](assets/test.png)  |  ![](assets/test3.png)

```c++
/* return: degree to add from horizontal depending on current angle (horizontal being the degree 0 for the sail on right side, and 180 on the left side)
function obtained via regression of experimental values*/
int degree_prediction_before_horizon(int boat_degree){ 
    if(boat_degree < 0){
        return 180-(int)(-0.5162*(-(double)(boat_degree))+94.3846);
    }else{
        return (int)((-0.5162*(double)(boat_degree)+94.3846));
    }
}
```

* Optimize the position with actual input from sensors
    * Test mutiple angle postitions
    * Mesure the actual speed of the boat
    * The idea of this method is to determine the best sail position experimentally each time by positioning the sail at close positions to the prediction and mesure the linear speed of the boat thanks to our IMU.
    * (cf `test_speed()` for IMU testing and `degree_sampling(int)` for general method)

```c++
//input: start_degree = initial guess optimal position returned by degre_prediction
//puts the sail on different positions to find the best speed and sets the optimal position;
void degree_sampling(int start_degree){
    double speeds[2*SPAN];
    for (int i = -SPAN; i < SPAN; i++)
    {
        sail.write(start_degree+2*i);
        delay(1000);
        speeds[i+SPAN] = test_speed();
        delay(500);
    }
    sail.write(start_degree + best_position(speeds));
    speed = 0;
}
```
## Turning methods
<mark> Inside the code there is multiple methods with the same functionnality, some are named `show_IMU` because they were used to showcase the function without using a windvane and measuring turning with the gyroscope of the IMU. In these methods we rely on a variable `angle_boat` that is modified through the turnings to keep track of the position of the boat with respect to the wind.<br> Add to this, the implementation of these methods force to change the use of the clock of the arduino. Therefore our methods of `localisation` and `show_IMU` are not compatible at the moment.</br>
In order to make this methods work together we should use another clock or update both whenever any of them are called.
 </mark>

 * The general method (cf `turning`) manages all maneuvers possibles, selecting the best fitting option for a given angle destination.
 * Input the final angle of the boat with respect to the wind and the program will chose which of the following methods to call.

  ### Turning on the same of the wind
 * if we don't want to change from side of the wind but get closer or further from the wind

 #### Basic description:

<div style="text-align: justify"> We compute the angle difference between our destination and our current position to decide the direction for the tiller. After this, we force the configuration to turn in the given direction and predict the position of the sail as smoothly as possible, as if a human was leaving the sail free to adapt to the turn. (cf <em>turning_settings</em>)</div> <br>

 ### Jibing
 * if we want to change from side of the wind with the wind in our backs

  <figure class="video_container">
  <iframe src="https://www.youtube.com/watch?v=JSpIAscG5cY" frameborder="0" allowfullscreen="false"> </iframe>
</figure>

 #### Basic description:

 <div style="text-align: justify"> We do a usual turn until a safe degree (around 150°) and then force a fast transition through the change of side of the wind by complete change of position of the sail (from one horizon to the other). During this transition we smoothly change the position of the sail via our prediction model.</div> <br>

 ### Tacking
 * if we want to cross the wind cone during a maneuver

 <figure class="video_container">
  <iframe src="https://www.youtube.com/watch?v=gMEOex9GQWU&t=55s" frameborder="0" allowfullscreen="false"> </iframe>
</figure>

 #### Basic description:
 <div style="text-align: justify"> Similar as what we do for jibing, we do a usual turn until a the edge of the windcone, stabilize there and after do a fast transition of the sail and the tiller to get through the windcone as fast as possible. We end by optimizing the position of the sail once the desired degree reached. </div> <br>

### Beating
 * if we want to go towards somewhere within the wind cone we will need to zigzag our way upwards the wind with mutiple tacks

 #### Basic description:
 <div style="text-align: justify"> Theory says that we should stay at the closest 45° of the wind until our destination is perpendicular to the end of the boat and tack to the otherside. We should zigzag our way up. This theory is not trivial with our components so we decidede to implement a simpler method. In function of which position inside the windcone we desire to get, we spend more or less time in one side of the windcone or the other(cf <em>fct_time</em>). After this we only reuse our tacking method for 5 times and consider it is sufficient(cf <em>beating()</em>). Using this method we should call it inside a loop until a certain position is attained.</div> <br>
 <br/><br/>

(here I show the decision function for turning, go inside each method of the code for more detail, if there is a similar function `show_IMU` it has more updated conditions and optimal functions but will be less modular)

```c++
/*
starting_angle: current direction of boat with respect to wind direction
desired_position: degree of the final position with respect to the wind
return: makes the boat turn to the given direction with a correct change for the sail and rudder, if possible (not in the wind cone)
*/
void turning(int starting_angle, int desired_position){
    if(degrees_limit(starting_angle) >= 0 && degrees_limit(desired_position) >= 180 - MAX_VAL_DEG_BOAT){
        //turning in the right side of the wind, without changing sides
        if(degrees_limit(desired_position)- degrees_limit(starting_angle) > 0){
            // we have to go further appart from the wind
            while(degree_boat() < degrees_limit(desired_position)){
                turning_settings(degrees_limit(desired_position)-degrees_limit(starting_angle),15);
            }
            Serial.println("end turning while");
            //turn finished at this point
           end_turn();

        }else if(degrees_limit(desired_position)- degrees_limit(starting_angle) < 0){
            //we have to get closer to the wind
            while(degree_boat() > degrees_limit(desired_position)){
                turning_settings(degrees_limit(desired_position)-degrees_limit(starting_angle),15);
            }
            //turn finished at this point
            Serial.println("end turning while");
            end_turn();
        }
    }else if(degrees_limit(starting_angle) <= 0 && degrees_limit(desired_position) <= -(180 - MAX_VAL_DEG_BOAT)){
        //turning in the left side of the wind
        if(degrees_limit(desired_position)- degrees_limit(starting_angle) > 0){
            // we have to get closer to the wind
            while(degree_boat() < degrees_limit(desired_position)){
                turning_settings(degrees_limit(desired_position)-degrees_limit(starting_angle),15);
            }
            //turn finished at this point
            Serial.println("end turning while");
            end_turn();

        }else if(degrees_limit(desired_position)- degrees_limit(starting_angle) < 0){
            //we have to get further appart from the wind
            while(degree_boat() > degrees_limit(desired_position)){
                turning_settings(degrees_limit(desired_position)-degrees_limit(starting_angle),15);
            }
            //turn finished at this point
            Serial.println("end turning while");
            end_turn();
        }
        
    }else{
        if(degrees_limit(starting_angle) <= 0 && degrees_limit(desired_position) >= (180 - MAX_VAL_DEG_BOAT)){
            if(degrees_limit(desired_position)<= 180+degrees_limit(starting_angle)){
                tacking(starting_angle,desired_position);
            }else{
                jibing(starting_angle,desired_position);
            }

        }else if(degrees_limit(starting_angle) >= 0 && degrees_limit(desired_position) <= -(180 - MAX_VAL_DEG_BOAT)){
            if(degrees_limit(desired_position) >= -(180-degrees_limit(starting_angle))){
                tacking(starting_angle,desired_position);
            }else{
                jibing(starting_angle,desired_position);
            }
        }else{
            beating(starting_angle,desired_position);
        }
    }
}
```

# Auxiliary useful methods

## Smooth sail and tiller calibration
 In order to not make the boat sink via too fast changes of position, we develop some methods to move the servomotors more carefully.
<br><br/>
 <mark>This method was only implemented during the last part of the project and therefore has only been used for the navigation without windvane. Implementation in methods with windvane input is required (remplace every servo.write by this method).</mark>

## Test of speed
Using IMU we obtain a linear acceleration that we try to stablize in order to mesure the actual speed of the boat.
```c++
// return: linear speed of the boat
double test_speed(){                 
    float acceleration = mpu.readNormalizeAccel().XAxis;
    delay(500);
    acceleration += mpu.readNormalizeAccel().XAxis;
    delay(500);
    acceleration += mpu.readNormalizeAccel().XAxis;
    acceleration = (acceleration / 3.0) - acc_drift;
    
    speed = (double)(acceleration * get_time()  + speed);
    return speed;
}
```

## Optimize sail position
Using the test speed we choose the best position of the sail experimentally each time, to correct any mistake made by the prediction method.
```c++
//input: start_degree = initial guess optimal position returned by degre_prediction
//puts the sail on different positions to find the best speed and sets the optimal position;
void degree_sampling(int start_degree){
    double speeds[2*SPAN];
    for (int i = -SPAN; i < SPAN; i++)
    {
        Serial.println(start_degree+2*i);
        sail.write(start_degree+2*i);
        delay(1000);
        speeds[i+SPAN] = test_speed();
        delay(500);
    }
    sail.write(start_degree + best_position(speeds));
    speed = 0;
    
}
```

## End turn
Settings of the program to use after any maneuver
```c++
//end for IMU
void end_turn_show_IMU(){
    smoother_angle_write_tiller(90);
    degree_sampling(degree_prediction_before_horizon(angle_boat));
}
```

## Get position of the boat
Method to read values from windvane if existing, to remodel to 360 if a correct windvane used.
```c++
//return: degree of the boat with respect to the wind in real time thanks to the windvane
int degree_boat(){
    return (int) ((double)analogRead(A0)/1023*180);
}
```

## Degree calculations
Making the degree span from -179° to 180° simplifies a lot calculations for turning
```c++
// return: degree between -179 and 180
int degrees_limit(int value){
    int res = value % 360;

    if(res<=180 && res>-180){
        return res;
    }else if(res<=-180){
        return (360+res);
    }else if(res>180){
        return -(360-res);
    }
    
}
```
