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

//position with IMU instead of windvane
int angle_boat = 0;

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
  delay(400); 
  acc_drift += mpu.readNormalizeAccel().XAxis;
  delay(400); 
  acc_drift += mpu.readNormalizeAccel().XAxis;
  acc_drift = acc_drift / 3.0;

  // choosing our desired destinations
  dest_total = 1;
  dests[0] = create_target(3.0, 5.0);

  //showing of turning
  angle_boat = degree_boat();
  //for smoother need always previous value
  sail.write(90);
  tiller.write(90);


}


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
        Serial.println(start_degree+2*i);
        sail.write(start_degree+2*i);
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

//more compact way of handle turning since alwayss the same computation
//handle when diff is more than 20° => if(diff>20){diff = 20}
void turning_settings(int diff,int time){
    if(diff > 20){
        smoother_angle_write_tiller(90-(19));
        delay(time);
        Serial.println(degree_boat());
        smoother_angle_write_sail(degree_prediction_before_horizon(degree_boat())); 
        delay(time);
    }else if(diff<-20){
        smoother_angle_write_tiller(90-(-19));
        delay(time);
        Serial.println(degree_boat());
        smoother_angle_write_sail(degree_prediction_before_horizon(degree_boat())); 
        delay(time);
    }else{
        smoother_angle_write_tiller(90-(diff));
        delay(time);
        Serial.println(degree_boat());
        smoother_angle_write_sail(degree_prediction_before_horizon(degree_boat())); 
        delay(time);
    }
}

void end_turn(){
    smoother_angle_write_tiller(90);
    degree_sampling(degree_prediction_before_horizon(degree_boat()));
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
    delay(500);
    acceleration += mpu.readNormalizeAccel().XAxis;
    delay(500);
    acceleration += mpu.readNormalizeAccel().XAxis;
    acceleration = (acceleration / 3.0) - acc_drift;
    
    speed = (double)(acceleration * get_time()  + speed);
    return speed;
}

//updates location of boat
Location location_update() {
   float acceleration = mpu.readNormalizeAccel().XAxis;
   float gyroscope = mpu.readNormalizeGyro().ZAxis;
   delay(500);
   acceleration += mpu.readNormalizeAccel().XAxis;
   gyroscope += mpu.readNormalizeGyro().ZAxis;
   delay(500);
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


int degree_boat_tacking(){
    return (int) map(analogRead(A0),0,1023,-90,90);
}




void turning_show_IMU(int desired_position){
    int diff = degrees_limit(desired_position)- degrees_limit(angle_boat);
    if(degrees_limit(angle_boat) >= 0 && degrees_limit(desired_position) >= 180 - MAX_VAL_DEG_BOAT){
        //turning in the right side of the wind, without changing sides
        if(diff > 0){
            // we have to go further appart from the wind
            double angle = 0.0;
            double time1 = get_time();
            while(angle <= diff){
                if(diff > 20){
                    tiller.write(90-(19));
                }else if(diff<-20){
                    tiller.write(90-(-19));
                }else{
                    tiller.write(90-(diff));
                }


                float gyroscope = mpu.readNormalizeGyro().ZAxis;
                delay(100);
                gyroscope += mpu.readNormalizeGyro().ZAxis;
                delay(100);
                gyroscope += mpu.readNormalizeGyro().ZAxis;

                angle += (double)(get_time()*(gyroscope/3.0));
                Serial.println(angle);
                smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
            }
            Serial.println("end turning while");
            angle_boat += angle;
            
            //or angle_boat= desired_position;
            Serial.println(angle_boat);
            //turn finished at this point
           end_turn_show_IMU();

        }else if(diff < 0){
            //we have to get closer to the wind
            double angle = 0.0;
            double time1 = get_time();
            while(angle >= diff){

                if(diff > 20){
                    tiller.write(90-(19));
                }else if(diff<-20){
                    tiller.write(90-(-19));
                }else{
                    tiller.write(90-(diff));
                }
                
                float gyroscope = mpu.readNormalizeGyro().ZAxis;
                delay(100);
                gyroscope += mpu.readNormalizeGyro().ZAxis;
                delay(100);
                gyroscope += mpu.readNormalizeGyro().ZAxis;

                angle += (double)(get_time()*(gyroscope/3.0));
                smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
            }
            Serial.println("end turning while");
            angle_boat += angle;
            //or angle_boat= desired_position;
            Serial.println(angle_boat);
            //turn finished at this point
           end_turn_show_IMU();
        }
    } else if(degrees_limit(angle_boat) <= 0 && degrees_limit(desired_position) <= -(180 - MAX_VAL_DEG_BOAT)){
        //turning in the left side of the wind
        if(diff > 0){
            // we have to get closer to the wind
            double angle = 0.0;
            double time1 = get_time();
            while(angle <= diff){
                if(diff > 20){
                    tiller.write(90-(19));
                }else if(diff<-20){
                    tiller.write(90-(-19));
                }else{
                    tiller.write(90-(diff));
                }


                float gyroscope = mpu.readNormalizeGyro().ZAxis;
                delay(250);
                gyroscope += mpu.readNormalizeGyro().ZAxis;
                delay(250);
                gyroscope += mpu.readNormalizeGyro().ZAxis;

                angle += (double)(get_time()*(gyroscope/3.0));
                smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
            }
            Serial.println("end turning while");
            angle_boat += angle;
            
            //or angle_boat= desired_position;
            Serial.println(angle_boat);
            //turn finished at this point
           end_turn_show_IMU();

        }else if(diff < 0){
            //we have to get further appart from the wind
            double angle = 0.0;
            double time1 = get_time();
            while(angle >= diff){
                    if(diff > 20){
                        tiller.write(90-(19));
                    }else if(diff<-20){
                        tiller.write(90-(-19));
                    }else{
                        tiller.write(90-(diff));
                    }
            float gyroscope = mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;

            angle += (double)(get_time()*(gyroscope/3.0));
            smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
            }

            Serial.println("end turning while");
            angle_boat += angle;
            //or angle_boat= desired_position;
            Serial.println(angle_boat);
            //turn finished at this point
           end_turn_show_IMU();
        
    }
}else{
        if(degrees_limit(angle_boat) <= 0 && degrees_limit(desired_position) >= (180 - MAX_VAL_DEG_BOAT)){
            if(degrees_limit(desired_position)<= 180+degrees_limit(angle_boat)){
                tacking_show_IMU(desired_position);
            }else{
                Serial.println("not implemented for IMU");
                Serial.flush();
            }

        }else if(degrees_limit(angle_boat) >= 0 && degrees_limit(desired_position) <= -(180 - MAX_VAL_DEG_BOAT)){
            Serial.println("entered in suposed tacking");
            Serial.println(degrees_limit(desired_position));
            Serial.flush();
            if(degrees_limit(desired_position) >= -(180-degrees_limit(angle_boat))){
                tacking_show_IMU(degrees_limit(desired_position));
            }else{
                Serial.println("not implemented for IMU");
                Serial.flush();
            }
        }else{
            Serial.println("not implemented for IMU");
            Serial.flush();
        }
    }
}

//end for IMU
void end_turn_show_IMU(){
    smoother_angle_write_tiller(90);
    degree_sampling(degree_prediction_before_horizon(angle_boat));
}

//tacking IMU
void tacking_show_IMU(int desired_position){
    int diff = 0;
    Serial.println("enter tacking");
    if(desired_position > 0){
        Serial.println("positive turn");
        //approach until -45 position
        diff = -(180 - MAX_VAL_DEG_BOAT) - degrees_limit(angle_boat);
        double angle = 0.0;
        double time1 = get_time();

        smoother_angle_write_tiller(90-19);

        while( angle <= diff){
            float gyroscope = mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;

            angle += (double)(get_time()*(gyroscope/3.0));
            Serial.println(angle);
            Serial.println("before -45°");
            Serial.flush();
            smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
        }
        smoother_angle_write_tiller(90);
        delay(1000);
        

        //reset of the mesures for turning
        angle = 0.0;
        time1 = get_time();

        smoother_angle_write_tiller(90-19);

        while( angle <= (180-MAX_VAL_DEG_BOAT) + degrees_limit(desired_position)){
            float gyroscope = mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;

            angle += (double)(get_time()*(gyroscope/3.0));
            Serial.println(angle);
            Serial.println("from -45 to pos");
            Serial.flush();
            smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
            
        }
        end_turn_show_IMU();
    }else{
        Serial.println("negative turn");
        //approach until -45 position
        diff = (180 - MAX_VAL_DEG_BOAT) - degrees_limit(angle_boat);
        double angle = 0.0;
        double time1 = get_time();

        smoother_angle_write_tiller(90+19);

        while( angle >= diff){
            float gyroscope = mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;

            angle += (double)(get_time()*(gyroscope/3.0));
            Serial.println(angle);
            Serial.println("before 45°");
            Serial.flush();
            smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
        }

        smoother_angle_write_tiller(90);
        delay(1000);
    

        //reset of the mesures for turning
        angle = 0.0;
        time1 = get_time();

        smoother_angle_write_tiller(90+19);

        tiller.write(90+19);
        while(angle >= -(180-MAX_VAL_DEG_BOAT)+ desired_position){
            float gyroscope = mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;
            delay(100);
            gyroscope += mpu.readNormalizeGyro().ZAxis;

            angle += (double)(get_time()*(gyroscope/3.0));
            Serial.println(angle);
            Serial.println("from 45 to position");
            Serial.flush();
            smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat+angle));
            
        }
        end_turn_show_IMU();
    }

}

void smoother_angle_write_sail(int angle){
    int current = sail.read();
    int count = angle - current;
    if(count>0){
        for (size_t i = 1; i <= count; i++)
        {
            sail.write(current+i);
            delay(10);
        }

    }else if(count <0){
        for (size_t i = 1; i <= -count; i++)
        {
            sail.write(current-i);
            delay(10);
        }
    }
}

void smoother_angle_write_tiller(int angle){
    int current = tiller.read();
    int count = angle - current;
    if(count>0){
        for (size_t i = 1; i <= count; i++)
        {
            tiller.write(current+i);
            delay(10);
        }

    }else if(count <0){
        for (size_t i = 1; i <= -count; i++)
        {
            tiller.write(current-i);
            delay(10);
        }
    }
}


//----------- end of the code -----------//

void loop(){
  
  /* tiller.write(90);
  sail.write(90);
  Serial.println("---degree---");
  Serial.println(degree_boat());
  delay(5000);
  Serial.println("---straight---");
  sail.write(degree_prediction_before_horizon(degree_boat()));
  delay(5000);
  Serial.println("---turning---");
  Serial.println(degree_boat());
  turning(degree_boat(),130);
  Serial.println("end turn");
  delay(1000);

  exit(0); */
  
  /*Serial.println("---degree boat---");
  Serial.println(degree_boat());
  Serial.println("---degree predicted for sail---");
  Serial.println(degree_prediction_before_horizon(degree_boat()));
  sail.write(degree_prediction_before_horizon(degree_boat()));*/
  
  /*
  Serial.println("--- BEGIN MOBILITY TEST TILLER ---");
  tiller.write(72);
  delay(1000);
  tiller.write(90);
  delay(1000);
  tiller.write(108);
  delay(3000);*/

  Serial.println("---BEGIN TEST TURNING IMU---");
  delay(1000);
  Serial.println("everything good");
  Serial.println(degree_boat());
  Serial.println("reset here if not done before, wind must be fixed before setup");
  delay(5000);
  Serial.println("--- INITIAL POSITION OF BOAT WITH RESPECT TO WIND ---");
  Serial.println(degree_boat());
  Serial.println("---straight---");
  smoother_angle_write_sail(degree_prediction_before_horizon(angle_boat));
  Serial.println("--- TURNING TOWARDS 100° ---");
  Serial.flush();
  turning_show_IMU(-100);
  Serial.println("END");
  
  exit(0);
  
 /* Serial.println("---new data---");
  Serial.println("linear speed :");
  Serial.println(test_speed());
  
  Serial.println("direction : ");
  location_update();
  Serial.println("r : ");
  Serial.println(loc.r);
  Serial.println("angle : ");
  Serial.println(loc.angle);
  Serial.println("x : ");
  Serial.println(loc.x);
  Serial.println("y : ");
  Serial.println(loc.y); 
  delay(4000); */
}
