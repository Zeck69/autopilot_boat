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
    
    ![](assets/test.png)  |  ![](assets/test3.png)

* Optimize the position with actual input from sensors
    * T

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