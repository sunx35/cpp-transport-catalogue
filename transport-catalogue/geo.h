#pragma once
#include <cmath>

// нужно ли использовать префикс std:: для математических функций, например sin, cos и т.д.
// не будет ли конфликта у разных копиляторов

namespace geo {

struct Coordinates {
    double latitude;
    double longitude;

    bool operator==(const Coordinates& other) const {
        return latitude == other.latitude && longitude == other.longitude;
    }

    bool operator!=(const Coordinates& other) const {
        return !operator==(other);
    }
};

// функция для вычисления расстояния между двумя точками
inline double ComputeDistance(Coordinates from, Coordinates to) {
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