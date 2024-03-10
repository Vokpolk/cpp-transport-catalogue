#include "json_reader.h"
#include <sstream>

namespace json_reader {
    using namespace std::literals;

    void Reader::InputJSON(std::istream& input, Catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer) {
        json::Document document = json::Load(input);
        auto& root = document.GetRoot();

        if (root.IsMap()) {
            const json::Dict& dict = root.AsMap();

            if (dict.count("base_requests"s)) {
                auto& base_request = dict.at("base_requests"s).AsArray();

                //описываю остановки
                for (auto& stop : base_request) {
                    auto& maybe_stop = stop.AsMap();
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
                    auto& maybe_bus = bus.AsMap();
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
                    auto& maybe_stop = stop.AsMap();
                    if (maybe_stop.count("type"s)) {
                        if (maybe_stop.at("type"s).AsString() == "Stop"s) {
                            std::vector<std::pair<std::string, double>> stop_and_distance;
                            std::string name = maybe_stop.at("name"s).AsString();
                            for (auto& s_and_d : maybe_stop.at("road_distances"s).AsMap()) {
                                catalogue.AddDistanceBetweenStops(catalogue.SearchStop(name), catalogue.SearchStop(s_and_d.first), s_and_d.second.AsDouble());
                            }
                        }
                    }
                }
            }
            //

            if (dict.count("render_settings")) {
                RenderSettings(dict, renderer);
            }

            //
            if (dict.count("stat_requests"s)) {
                auto& stat_requests = dict.at("stat_requests"s).AsArray();

                for (auto& request : stat_requests) {
                    auto& maybe_request = request.AsMap();
                    if (maybe_request.count("type"s)) {
                        if (maybe_request.at("type"s).AsString() == "Stop"s || maybe_request.at("type"s).AsString() == "Bus"s) {
                            stat_requests_.push_back({
                                maybe_request.at("id"s).AsInt(),
                                maybe_request.at("type"s).AsString(),
                                maybe_request.at("name"s).AsString() 
                            });
                        }
                        if (maybe_request.at("type"s).AsString() == "Map"s) {
                            stat_requests_.push_back({
                                maybe_request.at("id"s).AsInt(),
                                maybe_request.at("type"s).AsString(),
                                ""s
                                });
                        }
                    }
                }
            }
        }
    }

    void Reader::RenderSettings(const json::Dict& dict, renderer::MapRenderer& renderer) {
        auto& render_settings = dict.at("render_settings"s).AsMap();

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

    void Reader::OutputJSON(std::ostream& output, const Catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& map) {
        using namespace json;
        std::vector<json::Node> result_map;
        for (const auto& request : stat_requests_) {
            //если карта
            if (request.type == "Map"s) {
                std::ostringstream strs;
                map.OutputSVG(strs, catalogue);
                result_map.push_back(json::Dict{ {"map"s, strs.str()}, {"request_id"s, request.id}});
            }


            //если остановка
            if (request.type == "Stop"s) {
                auto buses_on_stop = catalogue.GetStopInfo(request.name);
                if (buses_on_stop.has_value()) {
                    json::Node buses_arr{ json::Array{buses_on_stop.value().begin(), buses_on_stop.value().end()} };
                    result_map.push_back(json::Dict{ {"buses"s, buses_arr}, {"request_id"s, request.id} });
                }
                else {
                    result_map.push_back(json::Dict{ {"request_id"s, request.id}, {"error_message"s, "not found"s}});
                }
                
            }
            //если маршрут
            if (request.type == "Bus"s) {

                auto buses = catalogue.GetRouteInfo(request.name);
                if (buses.has_value()) {
                    result_map.push_back(json::Dict{ 
                        {"curvature"s, buses.value().curvature},
                        {"request_id"s, request.id},
                        {"route_length"s, buses.value().new_length},
                        {"stop_count"s, static_cast<int>(buses.value().stops_on_route)},
                        {"unique_stop_count"s, static_cast<int>(buses.value().unique_stops)} 
                    });
                }
                else {
                    result_map.push_back(json::Dict{ {"request_id"s, request.id},
                                             {"error_message"s, "not found"s} });
                }
            }
        }

        PrintNode(result_map, output);
    }
}