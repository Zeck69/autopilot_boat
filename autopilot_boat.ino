#include <Wire.h>
#include <MPU6050.h>


MPU6050 mpu;

double speed;
unsigned long previous;
unsigned long present; 

//--- Code IMU -----


#include <stdlib.h>
#include <math.h>

#include <Servo.h>


#define MAX_VAL_DEG_BOAT 135.0 //maximum span for the sail to move from the winding cone of the boat 
//TODO TEST if 30° of winding cone is enough, not enought => 45°
#define MAX_VAL_DEG_SAIL 77.0 // degree span from l'horizontal line 
//TODO check this angle with DESIGNTEAM
#define SPAN 5 //size of span for testing degrees_limit

Servo sail;
Servo tiller;

void setup(){ 
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  sail.attach(9);
  tiller.attach(10);

  //IMU setup

  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  mpu.calibrateGyro();
  mpu.setThreshold(3);
  
  speed = 0.0;


}


// return: degree between -179 and 180
int degrees_limit(int value){
    int res = value % 360;

    if(res<=180){
        return res;
    }else{
        return -(360-res);
    }
    
}

// return: degree to add from horizontal depending on current angle (horizontal being the degree 0 for the sail on right side, and 180 on the left side)
// function obtained via regression of theoritical values (TODO improve)
int degree_prediction_before_horizon(int boat_degree){ 
    if(boat_degree < 0){
        return 180-(int)(-0.5162*(-(double)(boat_degree))+94.3846);
    }else{
        return (int)((-0.5162*(double)(boat_degree)+94.3846));
    }
    
}

//input: start_degree = initial guess optimal position returned by degre_prediction
//puts the sail on different positions to find the best speed and sets the optimal position;
void degree_sampling(int start_degree){
    double speeds[2*SPAN];
    for (size_t i = -SPAN; i < SPAN; i++)
    {
        sail.write(start_degree+i);
        delay(1000);
        speeds[i+SPAN] = test_speed();
        delay(500);
    }
    sail.write(start_degree + best_position(speeds));
    speed = 0;
    
}

/* speeds: array with 2*SPAN speeds of the boat
   return: index of best position for the sail, return ajusting value for the position*/
int best_position(double speeds[]){
    double max_speed = 0.0;
    int max = 0;
    for (size_t i =  -SPAN; i < SPAN; i++)
    {
        if(speeds[i+SPAN]>= max_speed){
            max_speed = speeds[i+SPAN];
            max=i;
        }
    }
    return max;
}

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
            //turn finished at this point
           end_turn();

        }else if(degrees_limit(desired_position)- degrees_limit(starting_angle) < 0){
            //we have to get closer to the wind
            while(degree_boat() > degrees_limit(desired_position)){
                turning_settings(degrees_limit(desired_position)-degrees_limit(starting_angle),15);
            }
            //turn finished at this point
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
            end_turn();

        }else if(degrees_limit(desired_position)- degrees_limit(starting_angle) < 0){
            //we have to get further appart from the wind
            while(degree_boat() > degrees_limit(desired_position)){
                turning_settings(degrees_limit(desired_position)-degrees_limit(starting_angle),15);
            }
            //turn finished at this point
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

//more compact way of handle turning since alwayss the same computation
//TODO add transition 
//handle when diff is more than 20° => if(diff>20){diff = 20}
void turning_settings(int diff,int time){
    if(diff > 20){
        tiller.write(90-(19));
        delay(time);
        sail.write(degree_prediction_before_horizon(degree_boat())); 
        delay(time);
    }else if(diff<-20){
        tiller.write(90-(-19));
        delay(time);
        sail.write(degree_prediction_before_horizon(degree_boat())); 
        delay(time);
    }else{
        tiller.write(90-(diff));
        delay(time);
        sail.write(degree_prediction_before_horizon(degree_boat())); 
        delay(time);
    }
}

void end_turn(){
    tiller.write(90);
    degree_sampling(degree_boat());
}

//turning mechanism for tacking
//usual turn until wind cone then force turning until outside the cone via simulating the movement of the sail and usual turning until end
void tacking(int starting_angle, int desired_position){
    if(desired_position > 0){
        turning(starting_angle,-(180-MAX_VAL_DEG_BOAT));
        while(degree_boat() < 180-MAX_VAL_DEG_BOAT){
            turning_settings(20,10);
        }
        turning(degree_boat(),desired_position);
    }else{
        turning(starting_angle,180-MAX_VAL_DEG_BOAT);
        while(degree_boat() > -(180-MAX_VAL_DEG_BOAT)){
            turning_settings(-20,10);
        }
        turning(degree_boat(),desired_position);
    }

}

//turning mechanism for jibing
void jibing(int starting_angle, int desired_position){
    if(desired_position>0){
        turning(starting_angle, -150);
        while(degree_boat() > -179){
            turning_settings(-20,5);
        }
        turning(degree_boat(),desired_position);

    }else{
        turning(starting_angle, 150);
        while(degree_boat() < 180){
            turning_settings(20,5);
        }
        turning(degree_boat(),desired_position);
    }

}

void beating(int starting_angle, int desired_position){
    if(starting_angle >= 180-MAX_VAL_DEG_BOAT){
        turning(starting_angle,180-MAX_VAL_DEG_BOAT);
    }else if(starting_angle <=-(180-MAX_VAL_DEG_BOAT)){
        turning(starting_angle, -(180-MAX_VAL_DEG_BOAT));
    }else if(starting_angle >= 0){
        while(degree_boat() < 180-MAX_VAL_DEG_BOAT){
            turning_settings(20,10);
        }
    }else if(starting_angle <= 0){
        while(degree_boat() > -(180-MAX_VAL_DEG_BOAT)){
            turning_settings(-20,10);
        }
    }
    //need 2 values saved for delays => right and left
    // fct = 1/2 + (1/90)*x pour répartir un temps lamdba
    //idéal: arriver jusqu'au pt ou t'es à 90° de ton objectif => no possible way
}


// return: linear speed of the boat
double test_speed(){ 
    float acceleration = mpu.readNormalizeAccel().XAxis;
    delay(250);
    acceleration += mpu.readNormalizeAccel().XAxis;
    delay(250);
    acceleration += mpu.readNormalizeAccel().XAxis;
    speed = (double)((acceleration/3.0) * get_time()  + speed);
    return speed;
}

typedef struct {
  double r = 0;
  double angle = 0;
  double x = 0;
  double y = 0;
} Direction;

Direction dir;

void direction_update() 
{
   float acceleration = mpu.readNormalizeAccel().XAxis;
   float gyroscope = mpu.readNormalizeGyro().ZAxis;
   delay(250);
   acceleration += mpu.readNormalizeAccel().XAxis;
   gyroscope += mpu.readNormalizeGyro().ZAxis;
   delay(250);
   acceleration += mpu.readNormalizeAccel().XAxis;
   gyroscope += mpu.readNormalizeGyro().ZAxis;
   
   acceleration = acceleration/3.0;
   gyroscope = gyroscope/3.0;
   
   double time = get_time();
   
   dir.r = (double) (acceleration * time * time  + speed * time + dir.r);
   dir.angle = (double) (gyroscope * time + dir.angle) ; 
   dir.x = dir.r*cos(dir.angle);
   dir.y = dir.r*sin(dir.angle);
   speed = (double)((acceleration/3.0) * time  + speed);
   
   return;
}

//return: time from last mesure
double get_time() {
  previous = present;
  present = millis();
  return (double)((present-previous) * 10^(-3));
}

//return: degree of the boat with respect to the wind in real time thanks to the windvane
//TODO add values how hand wind vane gives info => see method with interrupt for good rotary encoder 
int degree_boat(){
    return (int) ((double)analogRead(A0)/1023*180);
}


//----------- end of the code -----------//

void loop(){
  Serial.println("---BEGIN---");
  tiller.write(90);
  delay(1000);
  
  /*Serial.println("---degree boat---");
  Serial.println(degree_boat());
  Serial.println("---degree predicted for sail---");
  Serial.println(degree_prediction_before_horizon(degree_boat()));
  sail.write(degree_prediction_before_horizon(degree_boat()));*/
  
  /*tiller.write(72);
  delay(1000);
  tiller.write(90);
  delay(1000);
  tiller.write(108);
  delay(3000);*/
}
