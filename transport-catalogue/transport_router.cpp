#include "transport_router.h"

using namespace Router;

TransportRouter::TransportRouter(double bus_velocity, double bus_wait_time, const Catalogue::TransportCatalogue& catalogue)
    : router_settings_({ bus_velocity, bus_wait_time })
    , catalogue_(catalogue) {

    MakeGraphs();
}

double TransportRouter::ComputeWeightForRoute(const Catalogue::TransportCatalogue& catalogue,
                                              const domain::Bus& bus,
                                              const size_t first_stop_number,
                                              const size_t second_stop_number) const {

    double length = 0;//meters
    if (first_stop_number < second_stop_number) {
        for (size_t i = first_stop_number; i < second_stop_number; i++) {
            length += catalogue.GetDistanceBetweenStops(catalogue.SearchStop(bus.stops[i]), catalogue.SearchStop(bus.stops[i + 1]));
        }
    }
    else {
        for (size_t i = second_stop_number; i < first_stop_number; i++) {
            length += catalogue.GetDistanceBetweenStops(catalogue.SearchStop(bus.stops[i + 1]), catalogue.SearchStop(bus.stops[i]));
        }
    }

    return static_cast<double>((length * 60) / (router_settings_.bus_velocity * 1000));
}


void TransportRouter::FillStopsGraphs(graph::DirectedWeightedGraph<double>& graph_temp, size_t& edge_id) {
    //добавляем ребра ожиданий
    size_t stops_edges = 0;

    for (const auto& [stop_name, stop_info] : catalogue_.GetStops()) {
        stops_[stop_name] = { stops_edges, stops_edges + 1 };
        graph_temp.AddEdge(std::move(graph::Edge<double>{stops_edges, stops_edges + 1, router_settings_.bus_wait_time}));
        stops_edges += 2;
        edges_info_[edge_id] = WaitEdgeInfo{ router_settings_.bus_wait_time, stop_name };
        edge_id++;
    }
}
void TransportRouter::FillRoutesGraphs(graph::DirectedWeightedGraph<double>& graph_temp, size_t& edge_id) {
    for (const auto& [route_number, route] : catalogue_.GetBuses()) {

        if (!route.is_roundtrip) {
            for (size_t first_stop_number = 0; first_stop_number < route.stops.size() / 2 + 1; ++first_stop_number) {
                for (size_t second_stop_number = 0; second_stop_number < route.stops.size() / 2 + 1; ++second_stop_number) {
                    if (first_stop_number != second_stop_number) {
                        graph_temp.AddEdge(std::move(
                            graph::Edge<double>{stops_[route.stops[first_stop_number]].second,
                            stops_[route.stops[second_stop_number]].first,
                            ComputeWeightForRoute(catalogue_, route, first_stop_number, second_stop_number)}
                        ));

                        edges_info_[edge_id] = BusEdgeInfo{
                            (second_stop_number > first_stop_number ? second_stop_number - first_stop_number : first_stop_number - second_stop_number),
                            ComputeWeightForRoute(catalogue_, route, first_stop_number, second_stop_number), route_number
                        };
                        edge_id++;
                    }
                }

            }
        }
        else {
            for (size_t first_stop_number = 0; first_stop_number < route.stops.size() - 1; ++first_stop_number) {
                for (size_t second_stop_number = first_stop_number + 1; second_stop_number < route.stops.size(); ++second_stop_number) {
                    if (first_stop_number != second_stop_number) {
                        if (first_stop_number == 0 && second_stop_number == route.stops.size() - 1) {
                            graph_temp.AddEdge(std::move(graph::Edge<double>{
                                stops_[route.stops[first_stop_number]].second,
                                    stops_[route.stops[second_stop_number]].first,
                                    router_settings_.bus_wait_time
                            }));
                            edges_info_[edge_id] = BusEdgeInfo{ 0 , ComputeWeightForRoute(catalogue_, route, first_stop_number, second_stop_number), route_number };
                        }
                        else {
                            graph_temp.AddEdge(std::move(graph::Edge<double>{
                                stops_[route.stops[first_stop_number]].second,
                                    stops_[route.stops[second_stop_number]].first,
                                    ComputeWeightForRoute(catalogue_, route, first_stop_number, second_stop_number)
                            }));
                            edges_info_[edge_id] = BusEdgeInfo{
                                (second_stop_number > first_stop_number ? second_stop_number - first_stop_number : first_stop_number - second_stop_number),
                                ComputeWeightForRoute(catalogue_, route, first_stop_number, second_stop_number), route_number
                            };
                        }
                        edge_id++;
                    }
                }
            }
        }
    }
}

void TransportRouter::MakeGraphs() {
    //задаем размер вершин графа
    graph::DirectedWeightedGraph<double> graph_temp(catalogue_.GetStops().size() * 2);

    size_t edge_id = 0;

    //добавляем ребра ожиданий
    FillStopsGraphs(graph_temp, edge_id);
    //добавляем ребра маршрута
    FillRoutesGraphs(graph_temp, edge_id);

    graph_ = std::move(graph_temp);
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

  std::optional<std::deque<EdgeInfo>> TransportRouter::MakeRoute(const std::string& from, const std::string& to) const {

    if (!stops_.count(from) || !stops_.count(to)) {
        return std::nullopt;
    }

    route_info_ = router_->BuildRoute(stops_.at(from).first, stops_.at(to).first);
    if (!route_info_.has_value()) {
        return std::nullopt;
    }
    std::deque<EdgeInfo> result_edges_;

    for (const auto& edge : route_info_.value().edges) {
        result_edges_.push_back(edges_info_.at(edge));
    }

    return result_edges_;
}