#include <M5Stack.h>
#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

//#include <std_msgs/msg/int32.h>
#include <geometry_msgs/msg/twist.h>

rcl_publisher_t publisher;
//std_msgs__msg__Int32 msg;
geometry_msgs__msg__Twist msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

//#define LED_PIN 13
int sensorPin = 36; // set the input pin for the potentiometer.

int last_sensorValue = 100; // Stores the value last read by the sensor.
int cur_sensorValue = 0;  // Stores the value currently read by the sensor.

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


void error_loop(){
  M5.Lcd.print("Error!!\n");  
  //while(1){
  //digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  //delay(100);
  //}
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
    //msg.data++;
  }
}

void setup() {
  M5.begin();
  M5.Power.begin();
  pinMode(sensorPin, INPUT);
  dacWrite(25, 0);
  M5.Lcd.setTextSize(2); 
  M5.Lcd.print("Hello micro-ROS\n");
  M5.Lcd.print("publish Twist\n");
  
  set_microros_transports();
  
  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, HIGH);  
  
  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "turtle1/cmd_vel"));

  // create timer,
  const unsigned int timer_timeout = 1000;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  msg.linear.x = 0;
  msg.linear.y = 0;
  msg.linear.z = 0;
  msg.angular.x = 0;
  msg.angular.y = 0;
  msg.angular.z = 0;

}

void loop() {
  cur_sensorValue = analogRead(sensorPin);  // read the value from the sensor.
  M5.Lcd.setCursor(0, 40);  //Place the cursor at (0,25).
  if(abs(cur_sensorValue - last_sensorValue) > 10){ //debaunce
    M5.Lcd.fillRect(0, 40, 100, 40, BLACK);
    M5.Lcd.print(cur_sensorValue);
    last_sensorValue = cur_sensorValue;

    // angle value  LEFT MAX 4095 - 0 RIGHT MAX
    if (last_sensorValue < 1500) {  // Turn right 
      msg.angular.z = -(1500 - last_sensorValue) / 500.0;  //1.0;
    } else {
      if (last_sensorValue > 2500) { // Turn left
        msg.angular.z = (last_sensorValue - 2500) / 500.0; //-1.0;
      } else {
        msg.angular.z = 0.0;
      }
    }
    M5.Lcd.setCursor(0, 80);
    M5.Lcd.printf("linear x:%3.2f, y:%3.2f, z:%3.2f\n", msg.linear.x, msg.linear.y, msg.linear.z);  
    M5.Lcd.printf("angular x:%3.2f, y:%3.2f, z:%3.2f\n", msg.angular.x, msg.angular.y, msg.angular.z);
  }
  //delay(50);

  delay(100);
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
