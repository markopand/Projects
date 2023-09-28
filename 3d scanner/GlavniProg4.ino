#include <SPI.h>
#define sensor A0

//non adjustable variables
int steps_per_rotation_for_motor = 200; //Steps of a motor for a full circle
double distance_from_sensor_to_the_center=9; //  
int lead_screw_rotations_per_cm = 8;  
double degrees_per_step= 360/steps_per_rotation_for_motor;

//will change values later on
double RADIANS_per_one_step = 0.0; 
double current_angle=0;
double z_layer_height = 0.05;

int steps_z_height = 0; 
int z_loop=0;
float z = 0;
double y=0;
double x=0;

int z_loops_done=0;

int n=50;
float percentage=0.1; //10%
int step_delay = 1000; //in us     

//I/O 
int dir_z = 8;
int step_z = 7;
int dir_xy = 10;
int step_xy = 9;

void setup() {
  Serial.begin(115200);  // Set the baud rate to match the serial configuration on your PC
  delay(10000);  // Wait for the serial connection to establish
  pinMode(dir_xy, OUTPUT);
  pinMode(step_xy, OUTPUT);
  pinMode(dir_z, OUTPUT);
  pinMode(step_z, OUTPUT);
  RADIANS_per_one_step = (3.141592 / 180.0) * 1.8;
  steps_z_height = z_layer_height * steps_per_rotation_for_motor * lead_screw_rotations_per_cm;
  
  
}

void loop()
{
  int is_scanning_done;
  double data[200]; 
  for(int i = 0 ; i < steps_per_rotation_for_motor; i++){ //make a full circle while scanning every step
    Make_a_step_xy_motor();
    data[i]=measure_distance();  
  }
  
  if((is_scanning_done=calculate_xy_and_print(data))==1){
    return_to_starting_position(z_loops_done); // go back to the starting position( height)
    delay(2000000);
  }
  
  current_angle=0; 

  while(z_loop < steps_z_height) //move sensor up by 0.05cm in z direction
  {
  Make_a_step_z_motor_up();      
  z_loop = z_loop+1;              
  }
  
  z = z + z_layer_height;         
  z_loop = 0;                     
  z_loops_done++;
  if(z_loops_done==1){
    make_the_bottom_surface_flat(data);
  }

    }

double measure_distance(){
  double data[n];
  double average=0,post_filtering_avg=0;
  int i=0,  number_of_overshoot=0;

  while(i<n){
    float volts = analogRead(sensor)*0.00322580645;  // value from sensor * (5/1024)   0.00322580645
    data[i]= 13*pow(volts, -1); // worked out from datasheet graph
    average+=data[i];
    ++i;
  }
  average/=n;
  

  for(i=0;i<n;i++){
    if(data[i]>=12){
      data[i]=0;
      number_of_overshoot++;
    }
    post_filtering_avg+=data[i];
  }
  if(post_filtering_avg==0){
    
    return 0;
  }
  else{
  post_filtering_avg/=(n-number_of_overshoot);

  if(post_filtering_avg >= average*1.1 || post_filtering_avg <= average*0.9){
   
    return 0;
  }
  return post_filtering_avg;
 
}
}


int calculate_xy_and_print(double data[])
{
  int j=0;
 
  for(int i=0;i<steps_per_rotation_for_motor;i++){
    if(data[i]==0){
      j++;
    }
    else{
      
      data[i]=distance_from_sensor_to_the_center-data[i];    
      y =  (cos(current_angle) * data[i]);  
      x =  (sin(current_angle) * data[i]);
      Serial.println(String(x,3));
      Serial.println(String(y,3));
      Serial.println(String(z,2)); 
      
      current_angle+=RADIANS_per_one_step; 
    }        
  }
     if(j==steps_per_rotation_for_motor){ //check if whole item has been scanned
        return 1;
      }
      else{return 0;}
}

void Make_a_step_xy_motor(){
  
  digitalWrite(dir_xy,HIGH);
  digitalWrite(step_xy,HIGH);    //make a step
  delayMicroseconds(step_delay);
  digitalWrite(step_xy,LOW);
  delayMicroseconds(step_delay);
}

void Make_a_step_z_motor_up(){
  digitalWrite(dir_z,LOW);        //z_azis spin to right
  digitalWrite(step_z,HIGH);      //z_azis make a step
  delayMicroseconds(step_delay);
  digitalWrite(step_z,LOW);
  delayMicroseconds(step_delay);
}

void Make_a_step_z_motor_down(){
  digitalWrite(dir_z,HIGH);        //z_azis spin to right
  digitalWrite(step_z,HIGH);      //z_azis make a step
  delayMicroseconds(step_delay);
  digitalWrite(step_z,LOW);
  delayMicroseconds(step_delay);
}
void return_to_starting_position(int z_loops_done){
  int i,j;
  z_loops_done/=2.5;
  for(i=0;i<z_loops_done;i++){
    for(j=0;j<steps_per_rotation_for_motor;j++){
      Make_a_step_z_motor_down();
      
    }
  } 
}
void make_the_bottom_surface_flat(double data[]){
  int i,j,k;
 double  distance[200],percentage2=0.1;
 
 for(j=1;j<percentage2*100;j++){
   for(k=0;k<steps_per_rotation_for_motor;k++){
  distance[k]=data[k];
 } 
  for(i=0;i<steps_per_rotation_for_motor;i++){
    distance[i]*=(percentage2*j);
  }
  calculate_xyz_and_print_for_surface(distance);
 }
}

void calculate_xyz_and_print_for_surface(double data[])
{
  
  for(int i=0;i<steps_per_rotation_for_motor;i++){      
    y =  (cos(current_angle) * data[i]);  
    x =  (sin(current_angle) * data[i]);
    Serial.println(String(x,3));
    Serial.println(String(y,3));
    Serial.println(String(z,2)); 
    current_angle+=RADIANS_per_one_step; 
  } 
}
  
