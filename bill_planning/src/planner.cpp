#include "bill_planning/planner.hpp"

Planner::Planner()
{
    drivePoints = std::list<TilePosition>();
}

void Planner::setPubs(const ros::Publisher mp, const ros::Publisher fp, const ros::Publisher lp)
{
    _motor_pub = mp;
    _fan_pub = fp;
    _led_pub = lp;
}

void Planner::gridSearch(SensorReadings &sensorReadings)
{
    // Starting quadrants go:
    //   3   2
    //   4   1
    // Origin on bottom left

    int quadrant = 0;
    int startingX = sensorReadings.getCurrentTileX() < 4 ? 0 : 6;
    int startingY = sensorReadings.getCurrentTileY() < 4 ? 0 : 6;

    if (sensorReadings.getCurrentTileX() < 4)
    {
        quadrant = sensorReadings.getCurrentTileY() < 4 ? 4 : 3;

        startingY = quadrant == 3 ? 6 : 0;
        startingX = 0;
    }
    else
    {
        quadrant = sensorReadings.getCurrentTileY() < 4 ? 1 : 2;

        startingY = quadrant == 2 ? 6 : 0;
        startingX = 6;
    }

    // Drive to the first point
    publishDriveToTile(sensorReadings, startingX, startingY, 0.4);

    int xIncrement = (quadrant == 1) || (quadrant == 2) ? -1 : 1;

    for (int i = 0; i < 11; ++i)
    {
        if (i % 2 == 1)
        {
            startingX += xIncrement;
        }
        else if (i % 2 == 0)
        {
            startingY = startingY == 6 ? 0 : 6;
        }

        // Queue up the rest of the grid search points
        drivePoints.emplace_back(startingX, startingY);
    }
}

void Planner::cancelGridSearch(SensorReadings &sensorReadings)
{
    // Stop all driving
    publishStop();
    sensorReadings.setTargetHeading(-1);
    drivePoints.clear();
}

void Planner::publishStop()
{
    ROS_INFO("Commanding stop");
    _command_msg.command = bill_msgs::MotorCommands::STOP;
    _command_msg.heading = 0;
    _command_msg.speed = 0;
    _motor_pub.publish(_command_msg);
    is_moving = true;
}

void Planner::publishDrive(const int heading, const float speed)
{
    ROS_INFO("Commanding drive, heading: %i and speed: %f", heading, speed);
    _command_msg.command = bill_msgs::MotorCommands::DRIVE;
    _command_msg.heading = heading;
    _command_msg.speed = speed;
    _motor_pub.publish(_command_msg);
    is_moving = false;
}

void Planner::publishTurn(const int heading)
{
    ROS_INFO("Commanding turn, heading: %i", heading);
    _command_msg.command = bill_msgs::MotorCommands::TURN;
    _command_msg.heading = heading;
    _command_msg.speed = 0;  // Speed is hardcoded in the motor driver for turning
    _motor_pub.publish(_command_msg);
    is_moving = false;
}

void Planner::putOutFire()
{
    publishStop();

    // Turn ON/OFF Fan
    std_msgs::Bool cmd;
    cmd.data = true;
    _fan_pub.publish(cmd);  // Turn on fan
    ros::Duration(2).sleep();
    cmd.data = false;
    _fan_pub.publish(cmd);  // Turn off fan
}

void Planner::signalComplete()
{
    publishStop();
    std_msgs::Bool msg;
    msg.data = true;
    _led_pub.publish(msg);
    ros::Duration(2).sleep();
    msg.data = false;
    _led_pub.publish(msg);
}

void Planner::publishDriveToTile(SensorReadings &sensorReadings, const int x, const int y, const float speed, bool scanOnReach)
{
    int heading;
    int currentX = sensorReadings.getCurrentTileX();
    int currentY = sensorReadings.getCurrentTileY();

    // We will prioritize driving the longest leg of the horizontal/vertical drive first
    // If they are the same length, we will drive the vertical one first
    int xDistSq = (currentX - x)*(currentX - x);
    int yDistSq = (currentY - y)*(currentY - y);

    if (xDistSq == 0 && yDistSq == 0)
    {
        ROS_WARN("Attempting to drive to the same tile we are already on, bailing");
        return;
    }
    if (xDistSq <= yDistSq)
    {
        sensorReadings.setTargetPoint(currentX, y);

        drivePoints.emplace_back(currentX, y);
        sensorReadings.setTargetPoint(currentX, y);

        heading = currentY > y ? 270 : 90;

        if (xDistSq == 0)
        {
            // We only need one leg to drive to this point
            publishTurn(heading);
            sensorReadings.setTargetHeading(heading);
            return;
        }
    }
    else
    {
        sensorReadings.setTargetPoint(x, currentY);

        drivePoints.emplace_back(x,currentY);
        sensorReadings.setTargetPoint(x, currentY);
        heading = currentX > x ? 180 : 0;

        if (yDistSq == 0)
        {
            // We only need one leg to drive to this point
            publishTurn(heading);
            sensorReadings.setTargetHeading(heading);
            return;
        }
    }

    // Scan at the end of a drive to point if needed
    drivePoints.emplace_back(x,y, scanOnReach);

    // Publish a drive to the first queued point
    publishTurn(heading);
    sensorReadings.setTargetHeading(heading);
}

void Planner::driveAroundObstacle(SensorReadings &sensorReadings, bool takeLeft)
{
    int currentHeading = sensorReadings.getCurrentHeading();
    int targetHeading;

    // Facing 0
    if (currentHeading >= 0 && currentHeading < 90)
    {
        int currentY = sensorReadings.getCurrentTileY();
        int avoidanceY = takeLeft ? currentY + 1 : currentY - 1;
        int currentX = sensorReadings.getCurrentTileX();

        drivePoints.emplace_front(currentX + 2, currentY);
        drivePoints.emplace_front(currentX + 2, avoidanceY);
        drivePoints.emplace_front(currentX, avoidanceY);

        targetHeading = takeLeft ? 90 : 270;

    }
    // Facing 90
    else if(currentHeading >= 90 && currentHeading < 180)
    {
        int currentX = sensorReadings.getCurrentTileX();
        int avoidanceX = takeLeft ? currentX + 1 : currentX - 1;
        int currentY = sensorReadings.getCurrentTileY();

        drivePoints.emplace_front(currentX, currentY + 2);
        drivePoints.emplace_front(avoidanceX, currentY + 2);
        drivePoints.emplace_front(avoidanceX, currentY);

        targetHeading  = takeLeft ? 180 : 0;
    }
    // Facing 180
    else if (currentHeading >= 180 && currentHeading < 270)
    {
        int currentY = sensorReadings.getCurrentTileY();
        int avoidanceY = takeLeft ? currentY - 1 : currentY + 1;
        int currentX = sensorReadings.getCurrentTileX();

        drivePoints.emplace_front(currentX - 2, currentY);
        drivePoints.emplace_front(currentX - 2, avoidanceY);
        drivePoints.emplace_front(currentX, avoidanceY);

        targetHeading = takeLeft ? 270 : 90;
    }
    // Facing 270
    else
    {
        int currentX = sensorReadings.getCurrentTileX();
        int avoidanceX = takeLeft ? currentX + 1 : currentX - 1;
        int currentY = sensorReadings.getCurrentTileY();

        drivePoints.emplace_front(currentX, currentY + 2);
        drivePoints.emplace_front(avoidanceX, currentY + 2);
        drivePoints.emplace_front(avoidanceX, currentY);

        targetHeading  = takeLeft ? 180 : 0;
    }

    // Target the new point we just injected into the target queue
    sensorReadings.setTargetPoint(drivePoints.front().x, drivePoints.front().y);
    sensorReadings.setTargetHeading(targetHeading);
    publishTurn(targetHeading);
}

void Planner::scanTimerCallback(const ros::TimerEvent& event)
{
    is_scanning = false;
}

void Planner::ProcessNextDrivePoint(SensorReadings &sensorReadings)
{
    if (!drivePoints.empty())
    {
        int currentX = sensorReadings.getCurrentTileX();
        int currentY = sensorReadings.getCurrentTileY();

        // Make sure we really have arrived at the point we are aiming for
        TilePosition arrivedPosition = drivePoints.front();

        if (arrivedPosition.x != currentX || arrivedPosition.y != currentY)
        {
            ROS_WARN("We are assuming we have arrived at a point when we havent, stopping movement");
            sensorReadings.setTargetHeading(-1);
            publishStop();
            return;
        }

        bool scanOnReachLastPoint = drivePoints.front().scanOnReach;

        // Remove the front point from the queue, we have arrived at it.
        drivePoints.pop_front();

        if (!drivePoints.empty())
        {
            TilePosition nextLeg = drivePoints.front();
            int heading;

            // One of the two dimensions should always match
            if (currentX == nextLeg.x)
            {
                // Drive in Y
                heading = currentY > nextLeg.y ? 270 : 90;
            }
            else
            {
                // Drive in X
                heading = currentX > nextLeg.x ? 180 : 0;
            }

            sensorReadings.setTargetPoint(nextLeg.x, nextLeg.y);
            sensorReadings.setTargetHeading(heading);
            publishTurn(heading);
            return;
        }
        // Processed the last point in the list
        else
        {
            if (scanOnReachLastPoint)
            {
                is_scanning = true;

                // Timeout for scan
                ros::NodeHandle nh;
                ros::Timer scanTimeout = nh.createTimer(ros::Duration(2), &Planner::scanTimerCallback, this);

                // Scan at quarter driving speed
                publishDrive(sensorReadings.getCurrentHeading(), 0.16);
            }

            // Write an invalid target value
            // Maybe do this for position as well
            sensorReadings.setTargetHeading(-1);
            ROS_INFO("Completed drive to point");
            return;
        }
    }
}