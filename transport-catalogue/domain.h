#pragma once
#include <string>
#include <vector>

namespace domain {
    struct Stop {
        std::string name;   //название остановки
        double latitude;    //широта
        double longitude;   //долгота
    };

    struct Bus { //автобус и его маршрут
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip;
    };

    struct RouteInfo {
        std::size_t stops_on_route;
        std::size_t unique_stops;
        double length;
        double new_length;
        double curvature;
    };
}