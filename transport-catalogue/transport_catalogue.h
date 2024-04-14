#pragma once
#include <unordered_map>
#include <vector>
#include <deque>
#include <string_view>
#include <set>
#include <utility>
#include <optional>
#include "geo.h"
#include "domain.h"

#include <iostream>

namespace Catalogue {
    using namespace domain;

    struct Hasher {
        std::size_t operator()(std::pair<const Stop*, const Stop*> plate) const {
            auto hash1 = std::hash<const Stop*>{}(plate.first);
            auto hash2 = std::hash<const Stop*>{}(plate.second);

            if (hash1 != hash2) {
                return hash1 ^ hash2;
            }

            return hash1;
        }
    };

    class TransportCatalogue {
    public:
        void AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
        void AddStop(const std::string&, double lat, double lng);

        const Bus* SearchRoute(const std::string& route) const;
        const Stop* SearchStop(const std::string& stop) const;

        std::optional<RouteInfo> GetRouteInfo(const std::string& route) const;
        std::optional<const std::set<std::string>> GetStopInfo(const std::string& stop) const;

        //distance between stops
        void AddDistanceBetweenStops(const Stop* stop1, const Stop* stop2, double distance);
        double GetDistanceBetweenStops(const Stop& stop1, const Stop& stop2) const;

        const inline std::unordered_map<std::string, Stop>& GetStops() const { return stops_; }
        const inline std::unordered_map<std::string, Bus>& GetBuses() const { return buses_; }

        size_t GetDistanceCount() const {
            return distance_between_stops_.size();
        }

        double GetForwardDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;

    private:
        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<std::string, Bus> buses_;
        std::unordered_map<std::string, std::set<std::string>> buses_on_stops_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, double, Hasher> distance_between_stops_;

    };
}