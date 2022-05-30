#include <Wire.h>
#include <MPU6050.h>


#define MAX_STOPS 25

MPU6050 mpu;

typedef struct {   // initialized at 0
  double r = 0.0;
  double angle = 0.0;
  double x = 0.0;
  double y = 0.0;
}Location ;

double acc_drift = 0.0;
double speed = 0.0;

unsigned long previous;
unsigned long present;

Location loc;  
Location dest;
Location dests[MAX_STOPS];
int dest_index = 0; 
int dest_total; 

//--- Code IMU -----


#include <stdlib.h>
#include <math.h>

#include <Servo.h>


#define MAX_VAL_DEG_BOAT 135.0 //maximum span for the sail to move from the winding cone of the boat 
//TODO TEST if 30° of winding cone is enough, not enought => 45°
#define MAX_VAL_DEG_SAIL 77.0 // degree span from l'horizontal line 
//TODO check this angle with DESIGNTEAM
#define SPAN 5 //size of span for testing degrees_limit
#define DELAY 10000 //delay for beating square

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
  
   // compensating for drift
  acc_drift = mpu.readNormalizeAccel().XAxis;
  delay(200); 
  acc_drift += mpu.readNormalizeAccel().XAxis;
  delay(200); 
  acc_drift += mpu.readNormalizeAccel().XAxis;
  acc_drift = acc_drift / 3.0;

  // choosing our desired destinations
  dest_total = 1;
  dests[0] = create_target(3.0, 5.0);


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
// function obtained via regression of experimental values
//new prediction model (-0,496*boat_degree+90) => loss reduced from 4,312%
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
    for (int i = -SPAN; i < SPAN; i++)
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
    for (int i =  -SPAN; i < SPAN; i++)
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
    bool neg = false;
    if(starting_angle >= 180-MAX_VAL_DEG_BOAT){
        turning(starting_angle,180-MAX_VAL_DEG_BOAT);
    }else if(starting_angle <=-(180-MAX_VAL_DEG_BOAT)){
        turning(starting_angle, -(180-MAX_VAL_DEG_BOAT));
        neg = true;
    }else if(starting_angle >= 0){
        while(degree_boat() < 180-MAX_VAL_DEG_BOAT){
            turning_settings(20,10);
        }
        end_turn();
    }else if(starting_angle <= 0){
        while(degree_boat() > -(180-MAX_VAL_DEG_BOAT)){
            turning_settings(-20,10);
        }
        end_turn();
        neg = true;
    }

    if(neg){
        for (int i = 0; i < 5; i++)
        {
            delay((int)(DELAY*fct_time(desired_position)));
            turning(degree_boat(),180-MAX_VAL_DEG_BOAT);
            delay(DELAY - (int)(DELAY*fct_time(desired_position)));
            turning(degree_boat(), -(180-MAX_VAL_DEG_BOAT));
        }
        
    }else{
        for (int i = 0; i < 5; i++)
        {
            delay((int)(DELAY*fct_time(desired_position)));
            turning(degree_boat(), -(180-MAX_VAL_DEG_BOAT));
            delay(DELAY - (int)(DELAY*fct_time(desired_position)));
            turning(degree_boat(),180-MAX_VAL_DEG_BOAT);
        }
    }
    //idéal: arriver jusqu'au pt ou t'es à 90° de ton objectif => no possible way
}

double fct_time(int x){
    return 1/2 + (1/90)*(double)(abs(x));
}

//return: time from last mesure
double get_time() {
  previous = present;
  present = millis();
  return (double) ((present - previous) * pow(10, -3));
}


// return: linear speed of the boat
double test_speed(){                 
    float acceleration = mpu.readNormalizeAccel().XAxis;
    delay(250);
    acceleration += mpu.readNormalizeAccel().XAxis;
    delay(250);
    acceleration += mpu.readNormalizeAccel().XAxis;
    acceleration = (acceleration / 3.0) - acc_drift;
    
    speed = (double)(acceleration * get_time()  + speed);
    return speed;
}

//updates location of boat
Location location_update() {
   float acceleration = mpu.readNormalizeAccel().XAxis;
   float gyroscope = mpu.readNormalizeGyro().ZAxis;
   delay(250);
   acceleration += mpu.readNormalizeAccel().XAxis;
   gyroscope += mpu.readNormalizeGyro().ZAxis;
   delay(250);
   acceleration += mpu.readNormalizeAccel().XAxis;
   gyroscope += mpu.readNormalizeGyro().ZAxis;
   
   acceleration = (acceleration / 3.0) - acc_drift;
   gyroscope = gyroscope / 3.0;
   
   double time = get_time();
   
   loc.r = (double) (acceleration * time * time  + speed * time + loc.r);
   loc.angle = (double) (gyroscope * time + loc.angle) ; 
   loc.x = loc.r * cos(loc.angle * DEG_TO_RAD);
   loc.y = loc.r * sin(loc.angle * DEG_TO_RAD);
   speed = (double) (acceleration * time  + speed);
   
   return loc;
}

boolean arrival(){
  location_update();
  if (abs(loc.r - dest.r) <= 5 && abs(loc.angle - dest.angle) <= 5) {
    return true;
    }
  return false;
}

void update_arrival() {
  if (arrival() && dest_index < (dest_total - 1)) {
    dest = dests[ ++dest_index ];
  } else if (arrival() && dest_index == (dest_total - 1 )) {
    exit(0);
  }
}

Location create_target(double x, double y) {
  Location l; 
  l.x = x;
  l.y = y;
  l.r = sqrt(x*x + y*y) ;
  l.angle = atan2(y , x) * RAD_TO_DEG;  
  return l;
}


//return: degree of the boat with respect to the wind in real time thanks to the windvane
//TODO add values how hand wind vane gives info => see method with interrupt for good rotary encoder 
int degree_boat(){
    return (int) ((double)analogRead(A0)/1023*180);
}



//----------- end of the code -----------//

void loop(){
  Serial.println("---BEGIN---");
  sail.write(10);
  delay(1000);
  sail.write(50);
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
  
  /* Serial.println("---BEGIN---");
  Serial.println("time: ");
  Serial.println(get_time());
  Serial.println("acceleration :");
  Serial.println(mpu.readNormalizeAccel().XAxis);
  Serial.println("linear speed :");
  Serial.println(test_speed());
  Serial.println(" angular accel :");
  Serial.println(mpu.readNormalizeGyro().ZAxis);
  Serial.println("direction : ");
  location_update();
  Serial.println(loc.r);
  Serial.println(loc.angle);
  Serial.println(loc.x);
  Serial.println(loc.y); 
  delay(2000); */
}
