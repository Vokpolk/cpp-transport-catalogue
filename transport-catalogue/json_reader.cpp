#include "json_reader.h"
#include "json_builder.h"
#include <variant>
#include <sstream>

#include "graph.h"

namespace json_reader {
    using namespace std::literals;

    void Reader::InputJSON(std::istream& input, Catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer) {

        json::Document document = json::Load(input);
        auto& root = document.GetRoot();

        if (root.IsDict()) {
            const json::Dict& dict = root.AsDict();

            if (dict.count("base_requests"s)) {
                FillBaseRequests(dict, catalogue);
            }
            //
            if (dict.count("render_settings")) {
                RenderSettings(dict, renderer);
            }
            //
            if (dict.count("stat_requests"s)) {
                FillStatSettings(dict);
            }
            //
            if (dict.count("routing_settings"s)) {
                FillRouteSettings(dict);
            }
        }
    }

    void Reader::FillBaseRequests(const json::Dict& dict, Catalogue::TransportCatalogue& catalogue) {

        auto& base_request = dict.at("base_requests"s).AsArray();

        //описываю остановки
        for (auto& stop : base_request) {
            auto& maybe_stop = stop.AsDict();
            if (maybe_stop.count("type"s)) {
                if (maybe_stop.at("type"s).AsString() == "Stop"s) {
                    //заполняем остановки
                    catalogue.AddStop(
                        maybe_stop.at("name"s).AsString(),
                        maybe_stop.at("latitude"s).AsDouble(),
                        maybe_stop.at("longitude"s).AsDouble()
                    );

                }
            }
        }

        //описываю маршруты
        for (auto& bus : base_request) {
            auto& maybe_bus = bus.AsDict();
            if (maybe_bus.count("type"s)) {
                if (maybe_bus.at("type"s).AsString() == "Bus"s) {
                    //заполняем маршруты
                    std::vector<std::string> stops;
                    if (maybe_bus.count("stops"s)) {
                        auto& stops_arr = maybe_bus.at("stops"s).AsArray(); //массив остановок
                        for (auto& stop : stops_arr) {
                            stops.push_back(stop.AsString()); //первый проход
                        }
                    }
                    bool is_roundtrip = true;
                    if (!maybe_bus.at("is_roundtrip"s).AsBool()) {
                        auto arr = stops;
                        stops.insert(stops.end(), std::next(arr.rbegin()), arr.rend());
                        is_roundtrip = false;
                    }
                    catalogue.AddRoute(maybe_bus.at("name"s).AsString(), stops, is_roundtrip);
                }
            }
        }

        //добавляем расстояния между остановками
        for (auto& stop : base_request) {
            auto& maybe_stop = stop.AsDict();
            if (maybe_stop.count("type"s)) {
                if (maybe_stop.at("type"s).AsString() == "Stop"s) {
                    std::vector<std::pair<std::string, double>> stop_and_distance;
                    std::string name = maybe_stop.at("name"s).AsString();
                    for (auto& s_and_d : maybe_stop.at("road_distances"s).AsDict()) {
                        catalogue.AddDistanceBetweenStops(catalogue.SearchStop(name), catalogue.SearchStop(s_and_d.first), s_and_d.second.AsDouble());
                    }
                }
            }
        }

    }

    void Reader::RenderSettings(const json::Dict& dict, renderer::MapRenderer& renderer) {
        auto& render_settings = dict.at("render_settings"s).AsDict();

        renderer::RenderingStructure structure;

        structure.width_ = render_settings.at("width").AsDouble();
        structure.height_ = render_settings.at("height").AsDouble();
        structure.padding_ = render_settings.at("padding").AsDouble();
        structure.line_width_ = render_settings.at("line_width").AsDouble();
        structure.stop_radius_ = render_settings.at("stop_radius").AsDouble();
        structure.bus_label_font_size_ = render_settings.at("bus_label_font_size").AsInt();

        const auto& bus_label_offset = render_settings.at("bus_label_offset").AsArray();
        structure.bus_label_offset_.x = bus_label_offset[0].AsDouble();
        structure.bus_label_offset_.y = bus_label_offset[1].AsDouble();

        structure.stop_label_font_size_ = render_settings.at("stop_label_font_size").AsInt();
        const auto& stop_label_offset = render_settings.at("stop_label_offset").AsArray();
        structure.stop_label_offset_.x = stop_label_offset[0].AsDouble();
        structure.stop_label_offset_.y = stop_label_offset[1].AsDouble();

        if (render_settings.at("underlayer_color").IsArray()) {

            std::ostringstream strs;
            if (render_settings.at("underlayer_color").AsArray().size() == 4) {
                strs << "rgba("s;
                strs << render_settings.at("underlayer_color").AsArray()[0].AsInt();
                strs << ","s;
                strs << render_settings.at("underlayer_color").AsArray()[1].AsInt();
                strs << ","s;
                strs << render_settings.at("underlayer_color").AsArray()[2].AsInt();
                strs << ","s;
                strs << render_settings.at("underlayer_color").AsArray()[3].AsDouble();
                strs << ")"s;
                structure.underlayer_color_ = strs.str();
            }
            else {
                strs << "rgb("s;
                strs << render_settings.at("underlayer_color").AsArray()[0].AsInt();
                strs << ","s;
                strs << render_settings.at("underlayer_color").AsArray()[1].AsInt();
                strs << ","s;
                strs << render_settings.at("underlayer_color").AsArray()[2].AsInt();
                strs << ")"s;
                structure.underlayer_color_ = strs.str();
            }
        }
        else {
            structure.underlayer_color_ = render_settings.at("underlayer_color").AsString();
        }
        structure.underlayer_width_ = render_settings.at("underlayer_width").AsDouble();

        auto& color_palette = render_settings.at("color_palette").AsArray();

        for (auto& palette : color_palette) {

            svg::Color temp;

            if (palette.IsArray()) {
                std::ostringstream strs;
                if (palette.AsArray().size() == 4) {
                    strs << "rgba("s;
                    strs << palette.AsArray()[0].AsInt();
                    strs << ","s;
                    strs << palette.AsArray()[1].AsInt();
                    strs << ","s;
                    strs << palette.AsArray()[2].AsInt();
                    strs << ","s;
                    strs << palette.AsArray()[3].AsDouble();
                    strs << ")"s;
                    temp = strs.str();
                }
                else {

                    strs << "rgb("s;
                    strs << palette.AsArray()[0].AsInt();
                    strs << ","s;
                    strs << palette.AsArray()[1].AsInt();
                    strs << ","s;
                    strs << palette.AsArray()[2].AsInt();
                    strs << ")"s;
                    temp = strs.str();
                }
            }

            else {
                temp = palette.AsString();
            }
            structure.color_palette_.emplace_back(temp);
        }
        renderer.FillRenderingStructure(structure);
    }

    void Reader::FillRouteSettings(const json::Dict& dict) {
        auto& routing_settings = dict.at("routing_settings"s).AsDict();
        router_settings_.bus_velocity = routing_settings.at("bus_velocity"s).AsDouble();
        router_settings_.bus_wait_time = routing_settings.at("bus_wait_time"s).AsDouble();
        //router.SetSettings(routing_settings.at("bus_velocity"s).AsDouble(), routing_settings.at("bus_wait_time"s).AsDouble());

        //ну и здесь строим графы маршрутов.
        //router.MakeRouteGraph(catalogue);
    }

    void Reader::FillStatSettings(const json::Dict& dict) {
        auto& stat_requests = dict.at("stat_requests"s).AsArray();

        for (auto& request : stat_requests) {
            auto& maybe_request = request.AsDict();
            if (maybe_request.count("type"s)) {
                if (maybe_request.at("type"s).AsString() == "Stop"s || maybe_request.at("type"s).AsString() == "Bus"s) {
                    stat_requests_.push_back({
                        maybe_request.at("id"s).AsInt(),
                        maybe_request.at("type"s).AsString(),
                        maybe_request.at("name"s).AsString(),
                        ""s,
                        ""s
                        });
                }
                if (maybe_request.at("type"s).AsString() == "Map"s) {
                    stat_requests_.push_back({
                        maybe_request.at("id"s).AsInt(),
                        maybe_request.at("type"s).AsString(),
                        ""s,
                        ""s,
                        ""s
                        });
                }
                if (maybe_request.at("type"s).AsString() == "Route"s) {
                    stat_requests_.push_back({
                        maybe_request.at("id"s).AsInt(),
                        maybe_request.at("type"s).AsString(),
                        ""s,
                        maybe_request.at("from"s).AsString(),
                        maybe_request.at("to"s).AsString(),
                        });
                }
            }
        }
    }

    void Reader::OutputJSON(std::ostream& output, const Catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& map) {

        using namespace json;


        //создаем маршрут
        Router::TransportRouter router_(router_settings_.bus_velocity, router_settings_.bus_wait_time, catalogue);

        //создаем выходной json
        Builder builder;
        builder.StartArray();
        for (const auto& request : stat_requests_) {
            //если карта
            if (request.type == "Map"s) {
                std::ostringstream strs;
                map.OutputSVG(strs, catalogue);
                builder.StartDict().Key("map"s).Value(strs.str()).Key("request_id"s).Value(request.id).EndDict();
            }
            //если остановка
            if (request.type == "Stop"s) {
                auto buses_on_stop = catalogue.GetStopInfo(request.name);
                builder.StartDict();
                if (buses_on_stop.has_value()) {
                    json::Node buses_arr{ json::Array{buses_on_stop.value().begin(), buses_on_stop.value().end()} };
                    builder.Key("buses"s).Value(buses_arr.AsArray()).Key("request_id"s).Value(request.id);
                }
                else {
                    builder.Key("request_id"s).Value(request.id).Key("error_message"s).Value("not found"s);
                }
                builder.EndDict();
            }
            //если маршрут
            if (request.type == "Bus"s) {
                builder.StartDict();
                auto buses = catalogue.GetRouteInfo(request.name);
                if (buses.has_value()) {
                    builder
                        .Key("curvature"s).Value(buses.value().curvature)
                        .Key("request_id"s).Value(request.id)
                        .Key("route_length"s).Value(buses.value().new_length)
                        .Key("stop_count"s).Value(static_cast<int>(buses.value().stops_on_route))
                        .Key("unique_stop_count"s).Value(static_cast<int>(buses.value().unique_stops));
                }
                else {
                    builder.Key("request_id"s).Value(request.id).Key("error_message"s).Value("not found"s);
                }
                builder.EndDict();
            }
            //если построить маршрут от точки А до точки Б
            if (request.type == "Route"s) {
                builder.StartDict();
                if (auto result_situations = router_.MakeRoute(catalogue.SearchStop(request.from), catalogue.SearchStop(request.to))) {
                    builder.Key("items"s).StartArray();
                    double total_time = 0.0;
                    if (request.from != request.to) {
                        for (const auto& edge : result_situations.value()) {
                            builder.StartDict();
                            if (std::holds_alternative<Router::WaitEdgeInfo>(edge)) {

                                builder
                                    .Key("stop_name"s).Value(static_cast<std::string>(std::get<Router::WaitEdgeInfo>(edge).stop_name))
                                    .Key("time"s).Value(std::get<Router::WaitEdgeInfo>(edge).time).Key("type"s).Value("Wait"s);
                                total_time += std::get<Router::WaitEdgeInfo>(edge).time;
                            }
                            else {
                                builder
                                    .Key("bus"s).Value(static_cast<std::string>(std::get<Router::BusEdgeInfo>(edge).bus_name))
                                    .Key("span_count").Value(static_cast<int>(std::get<Router::BusEdgeInfo>(edge).span_count))
                                    .Key("time"s).Value(std::get<Router::BusEdgeInfo>(edge).time).Key("type"s).Value("Bus"s);
                                total_time += std::get<Router::BusEdgeInfo>(edge).time;
                            }
                            builder.EndDict();
                        }
                    }
                    builder.EndArray();
                    builder.Key("request_id"s).Value(request.id).Key("total_time"s).Value(total_time);
                }
                else {
                    builder.Key("request_id"s).Value(request.id).Key("error_message"s).Value("not found"s);
                }
                builder.EndDict();
            }
        }

        builder.EndArray();
        Document doc{ builder.Build() };
        Print(doc, output);
    }
}