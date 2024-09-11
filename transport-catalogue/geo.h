#pragma once

namespace geo {

struct Coordinates {
    double latitude;
    double longitude;

    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo