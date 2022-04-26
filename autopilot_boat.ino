#include <stdlib.h>
#include <math.h>

#define MAX_VAL_DEG_BOAT 150.0 //taille maximale depuis le cone deventement en degrees
#define MAX_VAL_DEG_SAIL 60.0 // degree span depuis l'horizontale
#define SPAN 5 //size of span for testing degrees

// return: degree between 0 and 360  => changed to -179 to 180
double degrees(double value){
    double res = floor(value);
    res = (double)((int)value % 360);
    while(res >360){
        res -= 360;
    }
    while(res<0){
        res+=360;
    }
    if(res<=180){
        return res;
    }else{
        return -(360-res);
    }
    
}

// return: angle à rajouter depuis l'horizontale en fonction de l'angle du bateau
double degree_prediction_before_horizon(double boat_degree){ 
    return (1/boat_degree)*MAX_VAL_DEG_SAIL/MAX_VAL_DEG_BOAT;
}

//puts the sail on different positions to find the best speed and sets the optimal position
void degree_sampling(double start_degree){
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
return: position
*/
double turning_rudder(double starting_angle, double desired_position){
    if(degrees(starting_angle) >= 0 && degrees(desired_position >= 0)){
        //turning in the right side of the wind
        return desired_position*0.5*test_speed();

    }else if(degrees(starting_angle) <= 0 && degrees(desired_position <= 0)){
        //turning in the left side of the wind
        return desired_position*90/test_speed();
    }else{
        jabing_tacking(starting_angle,desired_position);
    }
}

//TODO: turning mechanism for jabing or tacking
void jabing_tacking(double starting_angle, double desired_position){

}

//TODO: return direction of the boat with respect to the wind
double direction(){

    //this should get the direction of the wind, the wind will be consired to be at 0° always
    return 0.0;
}

//TODO return: linear speed of the boat
double test_speed(){ 

};
//end of the code