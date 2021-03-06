#include "bill_planning/sensor_readings.hpp"

SensorReadings::SensorReadings()
{
    //TODO PREPOPULATE QUEUE WITH POSSIBLE MAGNET POINTS
    _points_of_interest = std::queue<TilePosition>();

    // Start targets as invalid
    _target_heading = -1;
    _current_target_tile = TilePosition(-1,-1);
    //_home_tile.x = x;
    //_home_tile.y = y;
}

void SensorReadings::setStartRobotPerformanceThread(bool val)
{
    std::lock_guard<std::mutex> guard(_start_robot_performance_thread_mutex);
    _start_robot_performance_thread = val;
}

bool SensorReadings::getStartRobotPerformanceThread()
{
    std::lock_guard<std::mutex> guard(_start_robot_performance_thread_mutex);
    return _start_robot_performance_thread;
}

void SensorReadings::setUltraFwd(float val)
{
    std::lock_guard<std::mutex> guard(_ultra_fwd_mutex);
    _ultra_fwd = val;
}

float SensorReadings::getUltraFwd()
{
    std::lock_guard<std::mutex> guard(_ultra_fwd_mutex);
    return _ultra_fwd;
}

void SensorReadings::setUltraLeft(float val)
{
    std::lock_guard<std::mutex> guard(_ultra_left_mutex);
    _ultra_left = val;
}

float SensorReadings::getUltraLeft()
{
    std::lock_guard<std::mutex> guard(_ultra_left_mutex);
    return  _ultra_left;
}

void SensorReadings::setUltraRight(float val)
{
    std::lock_guard<std::mutex> guard(_ultra_right_mutex);
    _ultra_right = val;
}

float SensorReadings::getUltraRight()
{
    std::lock_guard<std::mutex> guard(_ultra_right_mutex);
    return _ultra_right;
}

void SensorReadings::setDetectedFireFwd(bool val)
{
    std::lock_guard<std::mutex> guard(_detected_fire_mutex);
    _detected_fire_fwd = val;
}

bool SensorReadings::getDetectedFireFwd()
{
    std::lock_guard<std::mutex> guard(_detected_fire_mutex);
    return  _detected_fire_fwd;
}
void SensorReadings::setDetectedFireLeft(bool val)
{
    std::lock_guard<std::mutex> guard(_detected_fire_mutex);
    _detected_fire_left = val;
}

bool SensorReadings::getDetectedFireLeft()
{
    std::lock_guard<std::mutex> guard(_detected_fire_mutex);
    return  _detected_fire_left;
}
void SensorReadings::setDetectedFireRight(bool val)
{
    std::lock_guard<std::mutex> guard(_detected_fire_mutex);
    _detected_fire_right = val;
}

bool SensorReadings::getDetectedFireRight()
{
    std::lock_guard<std::mutex> guard(_detected_fire_mutex);
    return  _detected_fire_right;
}

void SensorReadings::setCurrentHeading(int val)
{
    std::lock_guard<std::mutex> guard(_current_heading_mutex);
    _current_heading = val;
}

int SensorReadings::getCurrentHeading()
{
    std::lock_guard<std::mutex> guard(_current_heading_mutex);
    return _current_heading;
}

void SensorReadings::setTargetHeading(int val)
{
    std::lock_guard<std::mutex> guard(_target_heading_mutex);
    _target_heading = val;
}

int SensorReadings::getTargetHeading()
{
    std::lock_guard<std::mutex> guard(_target_heading_mutex);
    return _target_heading;
}

void SensorReadings::setCurrentTileX(int val)
{
    std::lock_guard<std::mutex> guard(_current_tile_mutex);
    _current_tile.x = val;
}

void SensorReadings::setCurrentTileY(int val)
{
    std::lock_guard<std::mutex> guard(_current_tile_mutex);
    _current_tile.y = val;
}

int SensorReadings::getCurrentTileX()
{
    std::lock_guard<std::mutex> guard(_current_tile_mutex);
    return _current_tile.x;
}

int SensorReadings::getCurrentTileY()
{
    std::lock_guard<std::mutex> guard(_current_tile_mutex);
    return _current_tile.y;
}

void SensorReadings::setTargetPoint(int x, int y)
{
    std::lock_guard<std::mutex> guard(_target_tile_mutex);
    _current_target_tile = TilePosition(x,y);
}

int SensorReadings::getTargetTileX()
{
    std::lock_guard<std::mutex> guard(_target_tile_mutex);
    return _current_target_tile.x;
}

int SensorReadings::getTargetTileY()
{
    std::lock_guard<std::mutex> guard(_target_tile_mutex);
    return _current_target_tile.y;
}

bool SensorReadings::isTargetTileValid()
{
    return _current_target_tile.x >= 0 && _current_target_tile.x <= 5 && _current_target_tile.y >=0 && _current_target_tile.y <= 5;
}

void SensorReadings::invalidateTargetTile()
{
    setTargetPoint(-1, -1);
}

void SensorReadings::setCurrentState(STATE val)
{
    std::lock_guard<std::mutex> guard(_current_state_mutex);
    _current_state = val;
}

STATE SensorReadings::getCurrentState()
{
    std::lock_guard<std::mutex> guard(_current_state_mutex);
    return _current_state;
}

void SensorReadings::setDetectionBit(unsigned char val)
{
    std::lock_guard<std::mutex> guard(_detection_bit_mutex);
    _detection_bit = val;
}

unsigned char SensorReadings::getDetectionBit()
{
    std::lock_guard<std::mutex> guard(_detection_bit_mutex);
    return _detection_bit;
}

void SensorReadings::setCurrentPositionX(float val)
{
    std::lock_guard<std::mutex> guard(_current_position_mutex);
    _current_position.x = val;
}

void SensorReadings::setCurrentPositionY(float val)
{
    std::lock_guard<std::mutex> guard(_current_position_mutex);
    _current_position.y = val;
}

float SensorReadings::getCurrentPositionX()
{
    std::lock_guard<std::mutex> guard(_current_position_mutex);
    return _current_position.x;
}

float SensorReadings::getCurrentPositionY()
{
    std::lock_guard<std::mutex> guard(_current_position_mutex);
    return _current_position.y;
}

bool SensorReadings::pointsOfInterestEmpty()
{
    std::lock_guard<std::mutex> guard(_points_of_interest_mutex);
    return _points_of_interest.empty();
}

int SensorReadings::pointsOfInterestSize()
{
    std::lock_guard<std::mutex> guard(_points_of_interest_mutex);
    return _points_of_interest.size();
}

TilePosition SensorReadings::pointsOfInterestFront()
{
    std::lock_guard<std::mutex> guard(_points_of_interest_mutex);
    return _points_of_interest.front();
}

void SensorReadings::pointsOfInterestPop()
{
    std::lock_guard<std::mutex> guard(_points_of_interest_mutex);
    _points_of_interest.pop();
}

// Prevent Duplicates
void SensorReadings::pointsOfInterestEmplace(TilePosition tp)
{
    std::pair <int,int> newVal (tp.x, tp.y);

    ROS_INFO("ATTEMPTING TO EMPLACE POINT");
    if (std::find(_s.begin(), _s.end(), newVal) == _s.end())
    {
        ROS_INFO("Adding point to queue x = %i, y = %i", tp.x, tp.y);
        std::lock_guard<std::mutex> guard(_points_of_interest_mutex);    
        _y_Objects.erase(std::remove(_y_Objects.begin(), _y_Objects.end(), tp.y), _y_Objects.end());
        _points_of_interest.emplace(tp);
        _s.push_back(newVal);
        // or "s.emplace(q.back());"
    }
}

int SensorReadings::freeRowTile()
{
    if (!_y_Objects.empty())
    {
        return _y_Objects.back();
    }
     return -1;
}

void SensorReadings::updateFlameTileFromLastSavedPoint(bool is_left)
{
    ROS_INFO("We found a fire to the left of right");
    int curr_x = getCurrentTileX();
    int curr_y = getCurrentTileY();

    if(!_s.empty())
    {
        ROS_INFO("POI's not empty, will try to set a flame tile");
        std::pair<int, int> tile_pos = _s.back();

        ROS_INFO("Current Position is x = %i, y = %i", curr_x, curr_y);
        ROS_INFO("Last Recorded point x = %i, y = %i", tile_pos.first, tile_pos.second);

        if(tile_pos.second == curr_y
            && ((is_left && tile_pos.first <= curr_x)
            || (!is_left && tile_pos.first >= curr_x)))
        {
            setFlameTileX(tile_pos.first);
            setFlameTileY(tile_pos.second);
        }
        else if (tile_pos.second != curr_y)
        {
            ROS_INFO("Last saved y is not where we detected a fire");
        }
        else
        {
            ROS_INFO("Last saved point is on the wrong side of the field");
        }
    }
}

void SensorReadings::setFlameTileX(int val)
{
    std::lock_guard<std::mutex> guard(_flame_tile_mutex);
    _flame_tile.x = val;
}

void SensorReadings::setFlameTileY(int val)
{
    std::lock_guard<std::mutex> guard(_flame_tile_mutex);
    _flame_tile.y = val;
}

int SensorReadings::getFlameTileX()
{
    std::lock_guard<std::mutex> guard(_flame_tile_mutex);
    return _flame_tile.x;
}

int SensorReadings::getFlameTileY()
{
    std::lock_guard<std::mutex> guard(_home_tile_mutex);
    return _flame_tile.y;
}

void SensorReadings::setHomeTile(int x, int y)
{
    std::lock_guard<std::mutex> guard(_home_tile_mutex);
    _home_tile.x = x;
    _home_tile.y = y;
}
int SensorReadings::getHomeTileX()
{
    std::lock_guard<std::mutex> guard(_home_tile_mutex);
    return _home_tile.x;
}
int SensorReadings::getHomeTileY()
{
    std::lock_guard<std::mutex> guard(_home_tile_mutex);
    return _home_tile.y;
}
