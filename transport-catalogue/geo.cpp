#include "geo.h"

#include <cmath>

namespace geo {

bool Coordinates::operator==(const Coordinates& other) const {
    return latitude == other.latitude && longitude == other.longitude;
}

bool Coordinates::operator!=(const Coordinates& other) const {
    return !operator==(other);
}

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.latitude * dr) * sin(to.latitude * dr)
        + cos(from.latitude * dr) * cos(to.latitude * dr) * cos(abs(from.longitude - to.longitude) * dr))
        * 6371000;
}

} // namespace geo