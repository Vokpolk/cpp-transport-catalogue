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
        double time = 0;
        std::string_view stop_name;
    };
    struct BusEdgeInfo {
        size_t span_count = 0;
        double time = 0;
        std::string_view bus_name;
    };

    using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;

    class TransportRouter {
    private:
        RouterSettings router_settings_;
        const Catalogue::TransportCatalogue& catalogue_;
        std::unordered_map<std::string, std::pair<size_t, size_t>> stops_;

        graph::DirectedWeightedGraph<double> graph_; //граф
        std::unique_ptr<graph::Router<double>> router_ = nullptr; //
        mutable std::optional<typename graph::Router<double>::RouteInfo> route_info_;

        std::unordered_map<graph::EdgeId, EdgeInfo> edges_info_;

        double ComputeWeightForRoute(
            const Catalogue::TransportCatalogue& catalogue,
            const domain::Bus& bus,
            const size_t first_stop_number,
            const size_t second_stop_number) const;

        void MakeGraphs();

        void FillStopsGraphs(graph::DirectedWeightedGraph<double>& graph_temp, size_t& edge_id);
        void FillRoutesGraphs(graph::DirectedWeightedGraph<double>& graph_temp, size_t& edge_id);

    public:

        TransportRouter(double bus_velocity, double bus_wait_time, const Catalogue::TransportCatalogue& catalogue);
        std::optional<std::deque<EdgeInfo>> MakeRoute(const std::string& from, const std::string& to) const;
    };
}