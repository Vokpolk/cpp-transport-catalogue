#include "transport_router.h"

using namespace Router;

double TransportRouter::ComputeWeightForRoute(const Catalogue::TransportCatalogue& catalogue,
                                              const domain::Bus& bus,
                                              const size_t s1,
                                              const size_t s2) const {

    double length = 0;//meters
    if (s1 < s2) {
        for (std::size_t i = s1; i < s2; i++) {
            length += catalogue.GetDistanceBetweenStops(*catalogue.SearchStop(bus.stops[i]), *catalogue.SearchStop(bus.stops[i + 1]));
        }
    }
    else {
        for (std::size_t i = s2; i < s1; i++) {
            length += catalogue.GetDistanceBetweenStops(*catalogue.SearchStop(bus.stops[i + 1]), *catalogue.SearchStop(bus.stops[i]));
        }
    }

    return static_cast<double>((length * 60) / (router_settings_.bus_velocity * 1000));
}

void TransportRouter::MakeRouteGraph(const Catalogue::TransportCatalogue& catalogue) {
    //задаем размер вершин графа
    size_ = catalogue.GetStops().size() * 2;
    graph::DirectedWeightedGraph<double> gr(size_);

    std::size_t edge_id = 0;

    //добавляем ребра ожиданий
    size_t i = 0;

    for (const auto& [key, value] : catalogue.GetStops()) {
        stops_[catalogue.SearchStop(key)] = { i, i + 1 };
        gr.AddEdge(std::move(graph::Edge<double>{i, i + 1, router_settings_.bus_wait_time}));
        i += 2;
        edges_info_[edge_id] = WaitEdgeInfo{ key , router_settings_.bus_wait_time };
        edge_id++;
    }
    //добавляем ребра маршрута
    for (const auto& [key, value] : catalogue.GetBuses()) {


        if (!value.is_roundtrip) {
            for (size_t i = 0; i < value.stops.size() / 2 + 1; i++) {
                for (size_t j = 0; j < value.stops.size() / 2 + 1; j++) {
                    if (i != j) {
                        gr.AddEdge(std::move(graph::Edge<double>{stops_[catalogue.SearchStop(value.stops[i])].second
                            , stops_[catalogue.SearchStop(value.stops[j])].first
                            , ComputeWeightForRoute(catalogue, value, i, j)})
                        );

                        edges_info_[edge_id] = BusEdgeInfo{ key , (j > i ? j - i : i - j) , ComputeWeightForRoute(catalogue, value, i, j) };
                        edge_id++;
                    }
                }

            }
        }
        else {
            for (size_t i = 0; i < value.stops.size() - 1; i++) {
                for (size_t j = i + 1; j < value.stops.size(); j++) {
                    if (i != j) {
                        if (i == 0 && j == value.stops.size() - 1) {
                            gr.AddEdge(std::move(graph::Edge<double>{stops_[catalogue.SearchStop(value.stops[i])].second
                                , stops_[catalogue.SearchStop(value.stops[j])].first
                                , router_settings_.bus_wait_time})
                            );
                            edges_info_[edge_id] = BusEdgeInfo{ key , 0 , ComputeWeightForRoute(catalogue, value, i, j) };
                        }
                        else {
                            gr.AddEdge(std::move(graph::Edge<double>{stops_[catalogue.SearchStop(value.stops[i])].second
                                , stops_[catalogue.SearchStop(value.stops[j])].first
                                , ComputeWeightForRoute(catalogue, value, i, j)})
                            );
                            edges_info_[edge_id] = BusEdgeInfo{ key , (j > i ? j - i : i - j) , ComputeWeightForRoute(catalogue, value, i, j) };
                        }
                        edge_id++;
                    }
                }
            }
        }
    }

    gr_ = std::move(gr);
    router_ = std::make_unique<graph::Router<double>>(gr_);
}

std::optional<std::deque<EdgeInfo>> TransportRouter::MakeRoute(const Catalogue::TransportCatalogue& catalogue, const std::string& from, const std::string& to) const {

    if (!stops_.count(catalogue.SearchStop(from)) || !stops_.count(catalogue.SearchStop(to))) {
        return std::nullopt;
    }

    first_wait_point_ = router_->BuildRoute(stops_.at(catalogue.SearchStop(from)).first, stops_.at(catalogue.SearchStop(from)).second);
    if (!first_wait_point_.has_value()) {
        return std::nullopt;
    }
    route_info_ = router_->BuildRoute(stops_.at(catalogue.SearchStop(from)).second, stops_.at(catalogue.SearchStop(to)).first);
    if (!route_info_.has_value()) {
        return std::nullopt;
    }
    std::deque<EdgeInfo> result_edges_;

    for (const auto& edge : first_wait_point_.value().edges) {
        result_edges_.push_back(edges_info_.at(edge));
    }
    for (const auto& edge : route_info_.value().edges) {
        result_edges_.push_back(edges_info_.at(edge));
    }

    return result_edges_;
}