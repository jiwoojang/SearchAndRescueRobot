#ifndef STATIC_VARIABLES_HPP
#define STATIC_VARIABLES_HPP

#include <queue>
#include "bill_planning/planner.hpp"

struct SensorReadings
{
    public:
        SensorReadings(Planner p);
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

        Planner planner;
        static std::queue<TilePosition> points_of_interest;
        //determinedPoints[0] small building, determinedPoints[1] large building
        static TilePosition determined_points [2];
    private:
};

#endif