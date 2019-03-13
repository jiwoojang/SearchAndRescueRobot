#include "ros/ros.h"
#include "tf/transform_datatypes.h"
#include "angles/angles.h"
#include "bill_msgs/MotorCommands.h"
#include "nav_msgs/Odometry.h"
#include "wiringPi.h"
#include <softPwm.h>
#include "bill_drivers/constant_definition.hpp"

const int PWM_RANGE = 100;  // Max pwm value
const int TURNING_SPEED = 30;
const float INT_CLAMP = 2.0;
const float dt = 0.1;  // TODO: Define this based off the frequency the odom channel gets published
const float MAX_VEL = 0.4;
float KP_HEADING;
float KI_HEADING;
float KP_SPEED;
float KI_SPEED;

bill_msgs::MotorCommands last_command_msg;
int last_heading = 90;
//float last_speed = 0;
//float speed_error_sum = 0;
float heading_error_sum = 0;

enum Direction
{
    CW = 1,
    CCW = -1
};

void stop()
{
    ROS_INFO("Stop");
    digitalWrite(MOTORA_FORWARD, LOW);
    digitalWrite(MOTORA_REVERSE, LOW);
    digitalWrite(MOTORB_FORWARD, LOW);
    digitalWrite(MOTORB_REVERSE, LOW);
    softPwmWrite(MOTORA_PWM, 0);
    softPwmWrite(MOTORB_PWM, 0);
}

void drive(const int left_cmd, const int right_cmd)
{
    ROS_INFO("Drive: Left = %i, Right = %i", left_cmd, right_cmd);

    if (right_cmd >= 0)
    {
        digitalWrite(MOTORA_FORWARD, HIGH);
        digitalWrite(MOTORA_REVERSE, LOW);
    }
    else
    {
        digitalWrite(MOTORA_FORWARD, LOW);
        digitalWrite(MOTORA_REVERSE, HIGH);
    }
    if (left_cmd >= 0)
    {
        digitalWrite(MOTORB_FORWARD, HIGH);
        digitalWrite(MOTORB_REVERSE, LOW);
    }
    else
    {
        digitalWrite(MOTORB_FORWARD, LOW);
        digitalWrite(MOTORB_REVERSE, HIGH);
    }
    softPwmWrite(MOTORA_PWM, std::abs(right_cmd));
    softPwmWrite(MOTORB_PWM, std::abs(left_cmd));

}

void turn(const Direction dir, const unsigned int speed)
{
    if (dir == CW)
    {
        ROS_INFO("Turning CW: Speed = %i", speed);
        digitalWrite(MOTORA_FORWARD, LOW);
        digitalWrite(MOTORA_REVERSE, HIGH);
        digitalWrite(MOTORB_FORWARD, HIGH);
        digitalWrite(MOTORB_REVERSE, LOW);
    }
    else
    {
        ROS_INFO("Turning CCW: Speed = %i", speed);
        digitalWrite(MOTORA_FORWARD, HIGH);
        digitalWrite(MOTORA_REVERSE, LOW);
        digitalWrite(MOTORB_FORWARD, LOW);
        digitalWrite(MOTORB_REVERSE, HIGH);
    }
    softPwmWrite(MOTORA_PWM, speed);
    softPwmWrite(MOTORB_PWM, speed);
}

void drivePI(int heading)
{
    // NOTE: Writing speed PI for now, although open loop speed control may be good enough for our purposes
    int heading_error = last_command_msg.heading - heading;
    // Keep heading error centered at 0 between -180 and 180
    if (heading_error > 180)
    {
        heading_error -= 360;
    }
    else if (heading_error < -180)
    {
        heading_error += 360;
    }

    /*
    float speed_error = last_command_msg.speed - speed;
    if (std::abs(speed_error_sum + speed_error * dt) < INT_CLAMP)
    {
        speed_error_sum += speed_error * dt;
    }
     */
    if (std::abs(heading_error_sum + heading_error * dt) < INT_CLAMP)
    {
        heading_error_sum += heading_error * dt;
    }
    //float speed_command = speed_error * KP_SPEED + speed_error_sum * KI_SPEED;

    float heading_command = heading_error * KP_HEADING + heading_error_sum * KI_HEADING;

    // Note: this PI calculation assumes forward motion, since the robot should never have to reverse
    // Except for construction check, but the errors will be 0 for said check

    // This calculation maps the desired speed between 0-100, then adds the speed PI correction and the heading
    // correction
    // Heading correction is subtracted for right wheel (positive heading command means clockwise turn)
    int right_speed = (int)((last_command_msg.speed / MAX_VEL) * PWM_RANGE + heading_command);
    int left_speed = (int)((last_command_msg.speed / MAX_VEL) * PWM_RANGE - heading_command);

    // Clamp speeds, bound checking but be sure to retain the ratio
    if (std::abs(right_speed) >= std::abs(left_speed))
    {
        float ratio = std::abs(float(right_speed) / left_speed);
        if (std::abs(right_speed) > PWM_RANGE)
        {
            right_speed = (int)(std::copysign(PWM_RANGE, right_speed));
            left_speed = (int)(std::copysign((right_speed / ratio), left_speed));  // Reduce other speed to match the other wheel
        }
    }
    else
    {
        float ratio = std::abs(float(left_speed) / right_speed);
        if (std::abs(left_speed) > PWM_RANGE)
        {
            left_speed = (int)(std::copysign(PWM_RANGE, left_speed));
            right_speed = (int)(std::copysign((left_speed / ratio), right_speed));  // Reduce other speed to match the other wheel
        }
    }
    drive(left_speed, right_speed);
}

void turningCallback(int heading)
{
    ROS_INFO("In Turning Callback");
    int error = last_command_msg.heading - heading;
    if ((error >= 0 && error <= 180) || (error < 0 && error >= -180))
    {
        // For now turn at a constant speed always
        turn(CW, TURNING_SPEED);
    }
    else
    {
        // For now turn at a constant speed always
        turn(CCW, TURNING_SPEED);
    }
    // Could add a condition here to stop turning if within a certain threshold, right now
    // We leave that up to the planner
}

void fusedOdometryCallback(const nav_msgs::Odometry::ConstPtr& msg)
{
    int heading = (int)(angles::to_degrees(tf::getYaw(msg->pose.pose.orientation)));
    last_heading = heading;

    if (last_command_msg.command == bill_msgs::MotorCommands::TURN)
    {
        turningCallback(heading);
    }
    else if (last_command_msg.command == bill_msgs::MotorCommands::DRIVE)
    {
        drivePI(heading);
    }
}

void motorCallback(const bill_msgs::MotorCommands::ConstPtr& msg)
{
    last_command_msg.command = msg->command;
    last_command_msg.heading = msg->heading;
    last_command_msg.speed = msg->speed;

    if (msg->command == bill_msgs::MotorCommands::STOP)
    {
        ROS_INFO("Calling Stop");
        stop();
    }
    else if (msg->command == bill_msgs::MotorCommands::TURN)
    {
        ROS_INFO("Calling Turn");
        // Call these here to start a robots action, callback from odom continues the action until completion
        turningCallback(last_heading);
    }
    else
    {
        ROS_INFO("Calling Drive");
        // Call these here to start a robots action, callback from odom continues the action until completion
        drivePI(last_heading);
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "motor_driver");
    wiringPiSetupGpio();
    pinMode(MOTORA_FORWARD, OUTPUT);
    pinMode(MOTORA_REVERSE, OUTPUT);
    pinMode(MOTORB_FORWARD, OUTPUT);
    pinMode(MOTORB_REVERSE, OUTPUT);
    pinMode(MOTORA_PWM, OUTPUT);
    pinMode(MOTORB_PWM, OUTPUT);
    softPwmCreate(MOTORA_PWM, 0, PWM_RANGE);
    softPwmCreate(MOTORB_PWM, 0, PWM_RANGE);

    ros::NodeHandle nh;
    ros::Subscriber sub_motor = nh.subscribe("motor_cmd", 1, motorCallback);
    ros::Subscriber sub_odom = nh.subscribe("fused_odometry", 1, fusedOdometryCallback);

    // Load parameters from yaml
    nh.getParam("/bill/motor_params/kp_heading", KP_HEADING);
    nh.getParam("/bill/motor_params/ki_heading", KI_HEADING);
    nh.getParam("/bill/motor_params/kp_speed", KP_SPEED);
    nh.getParam("/bill/motor_params/ki_speed", KI_SPEED);

    ros::spin();
    return 0;
}
