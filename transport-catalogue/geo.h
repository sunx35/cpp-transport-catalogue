#pragma once

// нужно ли использовать префикс std:: для математических функций, например sin, cos и т.д.
// не будет ли конфликта у разных копиляторов

namespace geo {

struct Coordinates {
    double latitude;
    double longitude;

    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

// функция для вычисления расстояния между двумя точками
double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo