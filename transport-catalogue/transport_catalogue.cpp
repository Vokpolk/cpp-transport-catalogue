#include "transport_catalogue.h"

Catalogue::RouteInfo Catalogue::TransportCatalogue::GetRouteInfo(std::string_view route) const {
	const auto& stops = buses_.at(route);

	std::set<std::string_view> unique;
	for (const auto& stop : stops.stops) {
		unique.insert(stop);
	}

	double length = 0;

	detail::Coordinates first;
	first.lat = stops_.at(buses_.at(route).stops.front()).latitude;
	first.lng = stops_.at(buses_.at(route).stops.front()).longitude;
	detail::Coordinates second;

	for (const auto& stop : buses_.at(route).stops) {
		second.lat = stops_.at(stop).latitude;
		second.lng = stops_.at(stop).longitude;
		length += ComputeDistance(first, second);
		first = second;
	}

	return RouteInfo{ buses_.at(route).stops.size(), unique.size(), length };
}