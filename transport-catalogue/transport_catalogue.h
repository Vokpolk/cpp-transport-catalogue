#pragma once
#include <unordered_map>
#include <vector>
#include <deque>
#include <string_view>
#include <set>
#include "geo.h"

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
	};

	class TransportCatalogue {
	public:
		void AddRoute(std::string_view name, std::vector<std::string_view> stops);
		void AddStop(std::string_view name, double lat, double lng);

		const Bus* SearchRoute(std::string_view route) const;
		const Stop* SearchStop(std::string_view stop) const;

		RouteInfo GetRouteInfo(std::string_view route) const;

		const std::set<std::string_view>& GetStopInfo(std::string_view stop) const;
	private:
		std::unordered_map<std::string_view, Stop> stops_;
		std::unordered_map<std::string_view, Bus> buses_;

		std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
	};
};