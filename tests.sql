-- Types Definitions
CREATE TYPE DSTATE AS ENUM('waiting', 'charging', 'monitoring');
-- Drone Table Creation and Clearing
CREATE TABLE IF NOT EXISTS drone (
    id SERIAL PRIMARY KEY,
    x FLOAT NOT NULL,
    y FLOAT NOT NULL,
    battery_autonomy INTERVAL,
    battery_life INTERVAL,
    max_autonomy INTERVAL,
    dstate DSTATE,
    last_update TIMESTAMP,
    CHECK(id > 0)
);
TRUNCATE TABLE drone;
-- Drone Creation
INSERT INTO drone (x, y, battery_autonomy, battery_life, dstate, last_update) VALUES (0, 0, 'hh:mm:ss'::INTERVAL, 'hh:mm:ss'::INTERVAL, 'waiting', CURRENT_TIMESTAMP);
-- Position Updating
UPDATE drone SET x = 0, y = 0, last_update = CURRENT_TIMESTAMP WHERE id = 1;
-- State Updating
UPDATE drone SET dstate = "waiting", last_update = CURRENT_TIMESTAMP WHERE id = 1;
-- Drone Updating
UPDATE drone SET x = 0, y = 0, autonomy_level = '00:00:00'::INTERVAL, max_autonomy = 'hh:mm:ss'::INTERVAL, dstate = 'waiting', last_update = CURRENT_TIMESTAMP WHERE id = 1;

