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
		void AddRoute(std::string_view name, std::vector<std::string_view> stops) {
			buses_[name] = { name, stops };
		}
		void AddStop(std::string_view name, double lat, double lng) {
			stops_[name] = { name, lat, lng };
		}
		void AddBusOnStop() {
			for (const auto& stop : stops_) {
				buses_on_stops_[stop.second.name];
			}
			for (const auto& bus : buses_) {
				for (const auto& stop : bus.second.stops) {

					buses_on_stops_[stop].insert(bus.second.name);
				}
			}
		}
		const Bus* SearchRoute(std::string_view route) const {
			if (buses_.count(route))
				return &buses_.at(route);
			else
				return nullptr;
		}
		const Stop* SearchStop(std::string_view stop) const {
			if (stops_.count(stop))
				return &stops_.at(stop);
			else
				return nullptr;
		}
		RouteInfo GetRouteInfo(std::string_view route) const;

		const std::set<std::string_view>& GetStopInfo(std::string_view stop) const {
			return buses_on_stops_.at(stop);
		}
	private:
		std::unordered_map<std::string_view, Stop> stops_;
		std::unordered_map<std::string_view, Bus> buses_;

		std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
	};
};