#include "transport_catalogue.h"

namespace Catalogue {

    void TransportCatalogue::AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip) {
        buses_[name] = { name, stops, is_roundtrip };
        for (const auto& stop : stops) {
            buses_on_stops_[&stops_[stop]].insert(name);
        }
    }
    void TransportCatalogue::AddStop(const std::string& name, double lat, double lng) {
        stops_[name] = { name, lat, lng };
        buses_on_stops_[&stops_[name]];
    }

    const Bus* TransportCatalogue::SearchRoute(const std::string& route) const {
        if (buses_.count(route))
            return &buses_.at(route);
        else
            return nullptr;
    }
    const Stop* TransportCatalogue::SearchStop(const std::string& stop) const {
        if (stops_.count(stop))
            return &stops_.at(stop);
        else
            return nullptr;
    }

    std::optional<RouteInfo> TransportCatalogue::GetRouteInfo(const std::string& route) const {
        if (!buses_.count(route)) {
            return std::nullopt;
        }
        const auto& stops = buses_.at(route);

        std::set<std::string> unique;
        for (const auto& stop : stops.stops) {
            unique.insert(stop);
        }

        double length = 0.0;

        geo::Coordinates first;
        first.lat = stops_.at(buses_.at(route).stops.front()).latitude;
        first.lng = stops_.at(buses_.at(route).stops.front()).longitude;
        geo::Coordinates second;

        const Stop* first_stop = &stops_.at(buses_.at(route).stops.front());
        const Stop* second_stop;
        double new_length = 0;

        for (const auto& stop : buses_.at(route).stops) {
            second_stop = &stops_.at(stop);

            new_length += GetDistanceBetweenStops(first_stop, second_stop);

            second.lat = stops_.at(stop).latitude;
            second.lng = stops_.at(stop).longitude;
            length += geo::ComputeDistance(first, second);
            first = second;
            first_stop = second_stop;
        }

        double curvature = new_length / length;
        return RouteInfo{ buses_.at(route).stops.size(), unique.size(), length, new_length, curvature };
    }

    std::optional<const std::set<std::string>> TransportCatalogue::GetStopInfo(const std::string& stop) const {
        if (!stops_.count(stop) || !buses_on_stops_.count(&stops_.at(stop))) {
            return std::nullopt;
        }
        return buses_on_stops_.at(&stops_.at(stop));
    }

    void TransportCatalogue::AddDistanceBetweenStops(const Stop* stop1, const Stop* stop2, double distance) {
        distance_between_stops_[{stop1, stop2}] = distance;
    }
    double TransportCatalogue::GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const {
        if (distance_between_stops_.count({ stop1, stop2 })) {
            return distance_between_stops_.at({ stop1, stop2 });
        }
        else if (distance_between_stops_.count({ stop2, stop1 })) {
            return distance_between_stops_.at({ stop2, stop1 });
        }
        else {
            return 0.0;
        }
    }

    double TransportCatalogue::GetForwardDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const {
        if (distance_between_stops_.count({ stop1, stop2 })) {
            return distance_between_stops_.at({ stop1, stop2 });
        }
        else {
            return 0.0;
        }
    }
}