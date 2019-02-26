#include "ros/ros.h"
#include "bill_msgs/MotorCommands.h"
#include "std_msgs/Bool.h"
#include "std_msgs/Float32.h"
#include "nav_msgs/Odometry.h"
#include <iostream>
#include "bill_planning/Enums.hpp"
#include "bill_planning/Planner.hpp"

struct State_Machine
{
    public:
        static int search_state;
        static Planner planner;

        State_Machine();
        static void AdvanceState();
        void ContinueAngularScan();
    private:
        const static int num_states = MachineStates::COUNT;
        const int scanning_angle = 10;
        static int previousDesiredHeading = 0;
}