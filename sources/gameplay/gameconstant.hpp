#pragma once

namespace Gameplay {
    constexpr float grid_size = 30.0f;
    constexpr float grid_altitude = 160.0f;
    constexpr float cloud_radius = grid_size / 4.0f;
    constexpr float touch_distance = grid_size / 3.0f;
    constexpr float pull_distance = grid_size / 3.0f;
    constexpr float pull_factor = 100.0f;
    constexpr float max_flying_speed = grid_size * 5.0f;
}