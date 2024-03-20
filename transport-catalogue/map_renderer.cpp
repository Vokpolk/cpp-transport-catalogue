#include "map_renderer.h"

namespace renderer {

    void MapRenderer::FillRoutes(std::vector<std::pair<std::string, Route>>& route,
        const Catalogue::TransportCatalogue& catalogue) {
        const auto& buses = catalogue.GetBuses();
        const auto& stops = catalogue.GetStops();

        for (const auto& bus : buses) {
            Route stops_and_coordinates;
            for (const auto& stop : bus.second.stops) {
                if (stops.count(stop)) {
                    stops_and_coordinates.stops.push_back({ stop, {stops.at(stop).latitude, stops.at(stop).longitude} });
                }
            }
            route.push_back({ bus.first, stops_and_coordinates });

        }
        std::sort(route.begin(), route.end(), [](std::pair<std::string, Route> name1, std::pair<std::string, Route> name2) {
            return (name1.first.compare(name2.first) < 0 ? true : false);
            });
    }


    void MapRenderer::FillRoutePolylines(std::vector<svg::Polyline>& routes_polylines,
        const std::vector<std::pair<std::string,
        Route>>&routes,
        SphereProjector& proj) {
        using namespace std::literals;
        std::size_t color_count = 0;
        for (const auto& route : routes) {
            if (color_count == structure_.color_palette_.size()) {
                color_count = 0;
            }

            if (route.second.stops.empty()) {
                ++color_count;
                continue;
            }
            svg::Polyline route_polyline;

            for (const auto& stop : route.second.stops) {

                route_polyline.AddPoint(proj({ stop.second.lat, stop.second.lng }))
                    .SetFillColor("none"s)
                    .SetStrokeColor(structure_.color_palette_[color_count])
                    .SetStrokeWidth(structure_.line_width_)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            }

            routes_polylines.push_back(route_polyline);
            ++color_count;
        }
    }


    void MapRenderer::FillRouteNames(std::vector<std::pair<std::string, Route>>& routes,
        const Catalogue::TransportCatalogue& catalogue,
        std::vector<svg::Text>& routes_names,
        SphereProjector& proj) {
        using namespace std::literals;
        svg::Text underlayer_route_name;
        svg::Text route_name;
        std::size_t color_count = 0;

        for (const auto& route : routes) {
            if (color_count == structure_.color_palette_.size()) {
                color_count = 0;
            }

            if (route.second.stops.empty()) {
                ++color_count;
                continue;
            }

            if (catalogue.SearchRoute(route.first)->is_roundtrip) {
                underlayer_route_name
                    .SetFillColor(structure_.underlayer_color_)
                    .SetStrokeColor(structure_.underlayer_color_)
                    .SetStrokeWidth(structure_.underlayer_width_)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetPosition(proj({ route.second.stops[0].second.lat, route.second.stops[0].second.lng }))
                    .SetOffset(structure_.bus_label_offset_)
                    .SetFontSize(structure_.bus_label_font_size_)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetData(route.first);

                route_name
                    .SetFillColor(structure_.color_palette_[color_count])
                    .SetPosition(proj({ route.second.stops[0].second.lat, route.second.stops[0].second.lng }))
                    .SetOffset(structure_.bus_label_offset_)
                    .SetFontSize(structure_.bus_label_font_size_)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetData(route.first);

                routes_names.emplace_back(std::move(underlayer_route_name));
                routes_names.emplace_back(std::move(route_name));
            }
            else {
                auto second_end_stop = (catalogue.GetRouteInfo(route.first).value().stops_on_route + 1) / 2;

                underlayer_route_name
                    .SetFillColor(structure_.underlayer_color_)
                    .SetStrokeColor(structure_.underlayer_color_)
                    .SetStrokeWidth(structure_.underlayer_width_)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetPosition(proj({ route.second.stops[0].second.lat, route.second.stops[0].second.lng }))
                    .SetOffset(structure_.bus_label_offset_)
                    .SetFontSize(structure_.bus_label_font_size_)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetData(route.first);

                route_name
                    .SetFillColor(structure_.color_palette_[color_count])
                    .SetPosition(proj({ route.second.stops[0].second.lat, route.second.stops[0].second.lng }))
                    .SetOffset(structure_.bus_label_offset_)
                    .SetFontSize(structure_.bus_label_font_size_)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetData(route.first);

                routes_names.emplace_back(std::move(underlayer_route_name));
                routes_names.emplace_back(std::move(route_name));

                if (second_end_stop - 1 > 0 && route.second.stops[0].first != route.second.stops[second_end_stop - 1].first) {

                    underlayer_route_name
                        .SetFillColor(structure_.underlayer_color_)
                        .SetStrokeColor(structure_.underlayer_color_)
                        .SetStrokeWidth(structure_.underlayer_width_)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                        .SetPosition(proj({ route.second.stops[second_end_stop - 1].second.lat, route.second.stops[second_end_stop - 1].second.lng }))
                        .SetOffset(structure_.bus_label_offset_)
                        .SetFontSize(structure_.bus_label_font_size_)
                        .SetFontFamily("Verdana"s)
                        .SetFontWeight("bold"s)
                        .SetData(route.first);

                    route_name
                        .SetFillColor(structure_.color_palette_[color_count])
                        .SetPosition(proj({ route.second.stops[second_end_stop - 1].second.lat, route.second.stops[second_end_stop - 1].second.lng }))
                        .SetOffset(structure_.bus_label_offset_)
                        .SetFontSize(structure_.bus_label_font_size_)
                        .SetFontFamily("Verdana"s)
                        .SetFontWeight("bold"s)
                        .SetData(route.first);

                    routes_names.emplace_back(std::move(underlayer_route_name));
                    routes_names.emplace_back(std::move(route_name));
                }
            }
            ++color_count;
        }
    }

    void MapRenderer::FillStopsCircles(std::vector<svg::Circle>& stop_circles,
        const std::vector<std::pair<std::string, Route>>& routes,
        SphereProjector& proj) {
        using namespace std::literals;
        svg::Circle stop_circle;
        std::set<std::pair<std::string, geo::Coordinates>> unique_stops;
        for (const auto& route : routes) {
            for (const auto& stop : route.second.stops) {
                unique_stops.insert(stop);
            }
        }

        for (const auto& unique_stop : unique_stops) {
            stop_circle
                .SetCenter(proj(unique_stop.second))
                .SetRadius(structure_.stop_radius_)
                .SetFillColor("white"s);
            stop_circles.push_back(stop_circle);
        }

    }

    void MapRenderer::FillStopsNames(std::vector<svg::Text>& stop_names,
        const std::vector<std::pair<std::string, Route>>& routes,
        SphereProjector& proj) {

        using namespace std::literals;
        svg::Text stop_name;
        svg::Text underlayer_stop_name;
        std::set<std::pair<std::string, geo::Coordinates>> unique_stops;
        for (const auto& route : routes) {
            for (const auto& stop : route.second.stops) {
                unique_stops.insert(stop);
            }
        }

        for (const auto& unique_stop : unique_stops) {

            underlayer_stop_name
                .SetFillColor(structure_.underlayer_color_)
                .SetStrokeColor(structure_.underlayer_color_)
                .SetStrokeWidth(structure_.underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetPosition(proj(unique_stop.second))
                .SetOffset(structure_.stop_label_offset_)
                .SetFontSize(structure_.stop_label_font_size_)
                .SetFontFamily("Verdana"s)
                .SetData(unique_stop.first);

            stop_name
                .SetFillColor("black"s)
                .SetPosition(proj(unique_stop.second))
                .SetOffset(structure_.stop_label_offset_)
                .SetFontSize(structure_.stop_label_font_size_)
                .SetFontFamily("Verdana"s)
                .SetData(unique_stop.first);

            stop_names.push_back(std::move(underlayer_stop_name));
            stop_names.push_back(std::move(stop_name));
        }
    }

    void MapRenderer::OutputSVG(std::ostream& out, const Catalogue::TransportCatalogue& catalogue) {
        using namespace std::literals;

        std::vector<std::pair<std::string, Route>> routes; //имя маршрута + набор остановок (имя + координаты)
        FillRoutes(routes, catalogue);

        std::vector<geo::Coordinates> geo;
        for (const auto& route : routes) {
            for (const auto& stop : route.second.stops) {
                geo.push_back({ stop.second.lat, stop.second.lng });
            }
        }

        SphereProjector proj(
            geo.begin(), geo.end(),
            structure_.width_, structure_.height_,
            structure_.padding_
        );

        svg::Document doc;

        std::vector<svg::Polyline> routes_polylines;
        FillRoutePolylines(routes_polylines, routes, proj);
        for (const auto& route_polyline : routes_polylines) {
            doc.Add(route_polyline);
        }

        std::vector<svg::Text> routes_names;
        FillRouteNames(routes, catalogue, routes_names, proj);
        for (const auto& route_name : routes_names) {
            doc.Add(route_name);
        }

        std::vector<svg::Circle> stop_circles;
        FillStopsCircles(stop_circles, routes, proj);
        for (const auto& stop_circle : stop_circles) {
            doc.Add(stop_circle);
        }

        std::vector<svg::Text> stop_names;
        FillStopsNames(stop_names, routes, proj);
        for (const auto& stop_name : stop_names) {
            doc.Add(stop_name);
        }

        doc.Render(out);
    }

}