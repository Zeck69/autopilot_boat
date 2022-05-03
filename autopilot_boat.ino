#include <stdlib.h>
#include <math.h>

#define MAX_VAL_DEG_BOAT 150.0 //maximum span for the sail to move from the winding cone of the boat 
//TODO TEST if 30° of winding cone is enough
#define MAX_VAL_DEG_SAIL 60.0 // degree span from l'horizontal line 
//TODO check this angle with DESIGNTEAM
#define SPAN 5 //size of span for testing degrees

//comments

// return: degree between -179 and 180
int degrees(int value){
    int res = value % 360;

    if(res<=180){
        return res;
    }else{
        return -(360-res);
    }
    
}

// return: degree to add from horizontal depending on current angle (horizontal being the degree 0 for the sail on right side, and 180 on the left side)
int degree_prediction_before_horizon(int boat_degree){ 
    return (int)(1.0/(double)boat_degree)*(MAX_VAL_DEG_SAIL/MAX_VAL_DEG_BOAT);
}

//input: start_degree = initial guess optimal position returned by degre_prediction
//puts the sail on different positions to find the best speed and sets the optimal position;
void degree_sampling(int start_degree){
    double speeds[2*SPAN];
    for (size_t i = -SPAN; i < SPAN; i++)
    {
        /* TODO: 
        - put position on servo
        - delay for some time delay(1000)?
        - test speed
        - save speed on the array
         */
    }
    int pos = start_degree + best_position(speeds);
    //TODO put the servo to this position
    
}

/* speeds: array with 2*SPAN speeds of the boat
   return: index of best position for the sail, return ajusting value for the position*/
int best_position(double speeds[]){
    double max_speed=0.0;
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
    if(degrees(starting_angle) >= 0 && degrees(desired_position) >= 180 - MAX_VAL_DEG_BOAT){
        //turning in the right side of the wind, without changing sides
        if(degrees(desired_position)- degrees(starting_angle) > 0){
            // we have to go further appart from the wind
            while(degree_boat() < degrees(desired_position)){
            /*TODO servos
            rudder = 90-(degrees(desired_position)-degrees(starting_angle));
            sail = degree_prediction_before_horizon(degree_boat());
            */ 
            }
            //turn finished at this point
            /*TODO servos
            rudder = 90;
            degree_sampling(degree_boat());
            */
        }else if(degrees(desired_position)- degrees(starting_angle) < 0){
            //we have to get closer to the wind
            while(degree_boat() > degrees(desired_position)){
            /*TODO servos
            rudder = 90-(degrees(desired_position)-degrees(starting_angle));
            sail = degree_prediction_before_horizon(degree_boat());
            */ 
            }
            //turn finished at this point
            /*TODO servos
            rudder = 90;
            degree_sampling(degree_boat());
            */
        }
    }else if(degrees(starting_angle) <= 0 && degrees(desired_position) <= -(180 - MAX_VAL_DEG_BOAT)){
        //turning in the left side of the wind
        if(degrees(desired_position)- degrees(starting_angle) > 0){
            // we have to get closer to the wind
            while(degree_boat() < degrees(desired_position)){
            /*TODO servos
            rudder = 90-(degrees(desired_position)-degrees(starting_angle));
            sail = degree_prediction_before_horizon(degree_boat());
            */ 
            }
            //turn finished at this point
            /*TODO servos
            rudder = 90;
            degree_sampling(degree_boat());
            */
        }else if(degrees(desired_position)- degrees(starting_angle) < 0){
            //we have to get further appart from the wind
            while(degree_boat() > degrees(desired_position)){
            /*TODO servos
            rudder = 90-(degrees(desired_position)-degrees(starting_angle));
            sail = degree_prediction_before_horizon(degree_boat());
            */ 
            }
            //turn finished at this point
            /*TODO servos
            rudder = 90;
            degree_sampling(degree_boat());
            */
        }
        
    }else{
        if(degrees(starting_angle) <= 0 && degrees(desired_position) >= (180 - MAX_VAL_DEG_BOAT)){
            if(degrees(desired_position)<= 180+degrees(starting_angle)){
                tacking(starting_angle,desired_position);
            }else{
                jibing(starting_angle,desired_position);
            }

        }else if(degrees(starting_angle) >= 0 && degrees(desired_position) <= -(180 - MAX_VAL_DEG_BOAT)){
            if(degrees(desired_position) >= -(180-degrees(starting_angle))){
                tacking(starting_angle,desired_position);
            }else{
                jibing(starting_angle,desired_position);
            }
        }
    }
}

//TODO: more compact way of handle turning since alwayss the same computation
//TODO handle when diff is more than 90° => if(diff>87){ diff = 87}
//TODO handle gentle transitions of degrees instead of crazy ass bitch changes for rudder
void turning_settings(int diff){

}

//TODO: turning mechanism for tacking
void tacking(int starting_angle, int desired_position){

}

//TODO: turning mechanism for jibing
void jibing(int starting_angle, int desired_position){

}


//TODO return: linear speed of the boat
double test_speed(){ 

};

//TODO return: degree of the boat with respect to the wind in real time thanks to the windvane
int degree_boat(){

}


//end of the code