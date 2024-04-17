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

        std::optional<RouteInfo*> GetRouteInfo(std::string_view route) const;
        std::optional<const std::set<std::string>*> GetStopInfo(std::string_view stop) const;

        //distance between stops
        void AddDistanceBetweenStops(const Stop* stop1, const Stop* stop2, double distance);
        double GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;

        const inline std::unordered_map<std::string_view, Stop*>& GetStops() const { return stops_; }
        const inline std::unordered_map<std::string_view, Bus*>& GetBuses() const { return buses_; }

        size_t GetDistanceCount() const {
            return distance_between_stops_.size();
        }

        double GetForwardDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;

    private:
        std::deque<Stop> all_stops_;
        std::unordered_map<std::string_view, Stop*> stops_;
        std::deque<Bus> all_buses_;
        std::unordered_map<std::string_view, Bus*> buses_;
        std::unordered_map<const Stop*, std::set<std::string>> buses_on_stops_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, double, Hasher> distance_between_stops_;

        //remember route info
        mutable std::unordered_map<std::string_view, RouteInfo> routes_info_;
    };
}