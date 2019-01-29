#include "ros/ros.h"
#include "std_msgs/Bool.h"

void setup()
{
    // TODO: Setup sensor
}

std_msgs::Bool read()
{
    // TODO: Read sensor

    // Create message to publish
    std_msgs::Bool msg;
    // TODO: Based on read data, determine what to publish

    return msg;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "ir_driver");
    ros::NodeHandle nh;
    ros::Publisher fire_pub = nh.advertise<std_msgs::Bool>("fire", 100);
    ros::Rate loop_rate(10);

    // Call sensor setup
    setup();

    while (ros::ok())
    {
        std_msgs::Bool msg = read();

        // Publish message, and spin thread
        fire_pub.publish(msg);
        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}
