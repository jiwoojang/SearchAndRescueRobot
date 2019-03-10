#ifndef SENSOR_READINGS_HPP
#define SENSOR_READINGS_HPP

#include <queue>
#include "bill_planning/planner.hpp"

class SensorReadings
{
    public:
        static int current_x_tile;
        static int current_y_tile;

        static int current_heading;

        static float ultra_fwd;
        static float ultra_left;
        static float ultra_right;

        static bool detected_fire;

        // WHEN SET TO TRUE, THE THREAD WILL START
        // ONLY SET WHEN WE HAVE RECEIVED FRONT, LEFT AND RIGHT ULTRASONIC READINGS
        static bool start_robot_performance_thread;

        static Planner planner;
        static std::queue<TilePosition> points_of_interest;

};

#endif