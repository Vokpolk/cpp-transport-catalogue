#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

    double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0.0;
        }
        const double earth_radius{ 6371000.0 };
        const double dr = M_PI / 180.0;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr)
            * cos(std::abs(from.lng - to.lng) * dr)) * earth_radius;
    }

}  // namespace geo