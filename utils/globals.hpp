#ifndef GLOBAL_HPP
#define GLOBAL_HPP

enum DroneState {
    CHARGING,    ///< Charging in Tower.
    READY,       ///< Full battery, ready to be assigned to a block.
    WAITING,     ///< Waiting a new location from tower.
    MONITORING,  ///< Drone is monitoring a cell.
    RETURNING,   ///< Drone is returning to tower(low battery).
    DEAD         ///< Used from tower to decide if a drone is no longer available (fault).
};

// 10 * sqrt(2) = 14.14 -> Approximate to 15 so each drone is secure to control completily one single cell
#define GRID_FACTOR 15 ///< Size of an Area cell in meters. Each cell is big GRID_FACTOR meters. 

#endif  // GLOBAL_HPP