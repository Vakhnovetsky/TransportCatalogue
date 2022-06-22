#pragma once

namespace geo {

    struct Coordinates {
        double lat = 0; // Широта
        double lng = 0; // Долгота
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
