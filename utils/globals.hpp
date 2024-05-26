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

#endif  // GLOBAL_HPP