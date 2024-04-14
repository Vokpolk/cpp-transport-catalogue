#pragma once
#include "graph.h"
#include "transport_catalogue.h"
#include "router.h"
#include <vector>
#include <map>
#include <variant>
#include <memory>

namespace Router {

    struct RouterSettings {
        double bus_velocity;  //скорость автобуса, в км/ч
        double bus_wait_time; //время ожидания автобуса на остановке, в минутах
    };

    enum class TYPE {
        WAIT,
        BUS
    };
    
    struct WaitEdgeInfo {
        std::string_view stop_name;
        double time = 0;
    };
    struct BusEdgeInfo {
        std::string_view bus_name;
        size_t span_count = 0;
        double time = 0;
    };

    using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;

    class TransportRouter {
    private:
        RouterSettings router_settings_;
        size_t size_;

        std::unordered_map<const domain::Stop*, std::pair<size_t, size_t>> stops_;

        graph::DirectedWeightedGraph<double> gr_; //граф
        std::unique_ptr<graph::Router<double>> router_ = nullptr; //

        mutable std::optional<typename graph::Router<double>::RouteInfo> first_wait_point_;
        mutable std::optional<typename graph::Router<double>::RouteInfo> route_info_;

        //using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;
        std::unordered_map<graph::EdgeId, EdgeInfo> edges_info_;

        double ComputeWeightForRoute(const Catalogue::TransportCatalogue& catalogue,
            const domain::Bus& bus,
            const size_t s1,
            const size_t s2) const;

    public:

        void SetSettings(double bus_velocity, double bus_wait_time) {
            router_settings_ = { bus_velocity, bus_wait_time };
        }


        void MakeRouteGraph(const Catalogue::TransportCatalogue& catalogue);

        std::optional<std::deque<EdgeInfo>> MakeRoute(const Catalogue::TransportCatalogue& catalogue, const std::string& from, const std::string& to) const;
    };
}