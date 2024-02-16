#pragma once
#include <unordered_map>
#include <vector>
#include <deque>
#include <string_view>
#include <set>
#include <utility>
#include "geo.h"

#include <iostream>

namespace Catalogue {
    struct Stop {
        std::string_view name;   //название остановки
        double latitude;    //широта
        double longitude;   //долгота
    };

    struct Bus { //автобус и его маршрут
        std::string_view name;
        std::vector<std::string_view> stops;
    };

    struct RouteInfo {
        std::size_t stops_on_route;
        std::size_t unique_stops;
        double length;
        double new_length;
        double curvature;
    };

    struct Hasher {
        std::size_t operator()(std::pair<const Stop*, const Stop*> plate) const {


            auto hash1 = std::hash<const Stop*>{}(plate.first);
            auto hash2 = std::hash<const Stop*>{}(plate.second);

            //std::cout << "Stop1.name: " << plate.first->name << " Stop1.name: " << plate.second->name << "\n";
            //std::cout << "Stop1* - " << plate.first << "; Stop2* - " << plate.second << "\n";
            //std::cout << "hash1* - " << hash1 << "; hash1 ^ hash2* - " << (hash1 ^ hash2) << "\n";

            if (hash1 != hash2) {
                return hash1 ^ hash2;
            }

            return hash1;
        }
    };

    class TransportCatalogue {
    public:
        void AddRoute(std::string_view name, std::vector<std::string_view> stops);
        void AddStop(std::string_view name, double lat, double lng);

        const Bus* SearchRoute(std::string_view route) const;
        const Stop* SearchStop(std::string_view stop) const;

        RouteInfo GetRouteInfo(std::string_view route) const;

        const std::set<std::string_view>& GetStopInfo(std::string_view stop) const;

        //distance between stops
        void AddDistanceBetweenStops(const Stop* stop1, const Stop* stop2, double distance);
        double GetDistanceBetweenStops(const Stop& stop1, const Stop& stop2) const;

    private:
        std::unordered_map<std::string_view, Stop> stops_;
        std::unordered_map<std::string_view, Bus> buses_;

        std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;

        std::unordered_map<std::pair<const Stop*, const Stop*>, double, Hasher> distance_between_stops_;

    };
};