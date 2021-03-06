#ifndef PLANNER_HPP
#define PLANNER_HPP

#include "std_msgs/Bool.h"
#include "std_msgs/Float32.h"
#include "nav_msgs/Odometry.h"
#include <iostream>
#include <queue>
#include "bill_msgs/MotorCommands.h"
#include "bill_planning/position.hpp"
#include "bill_planning/sensor_readings.hpp"
#include "bill_planning/graph_path.hpp"
#include <list>

class Planner
{
  public:
    Planner();
    void setPubs(ros::Publisher mp, ros::Publisher fp, ros::Publisher lp);
    void publishStop();
    void publishDrive(int heading, float speed);
    void publishTurn(int heading);
    void putOutFire();
    void signalComplete();

    void publishDriveToTile(SensorReadings &sensorReadings, int x, int y, float speed, bool scanOnReach = false);
    void cancelDriveToTile(SensorReadings &sensorReadings);

    void driveAroundObstacle(SensorReadings &sensorReadings);

    bool is_moving = false;

    void setIsScanning(bool val);
    bool getIsScanning();

    // This function returns a pointer so that we can return a nullptr if the queue is empty
    void ProcessNextDrivePoint(SensorReadings &sensorReadings);
    bool isDrivePointsEmpty();

  private:
    std::mutex _is_scanning_mutex;
    bool _is_scanning = false;

    void scanTimerCallback(const ros::TimerEvent& event);
    bill_msgs::MotorCommands _command_msg;
    ros::Publisher _motor_pub;
    ros::Publisher _fan_pub;
    ros::Publisher _led_pub;
    std::list<TilePosition> drivePoints;

    GraphPath graphPath;

    float driveSpeed = 0;
};

#endif
