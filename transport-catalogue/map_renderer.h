#pragma once

#include <optional>
#include <vector>
#include <tuple>
#include <algorithm>
#include <variant>
#include "svg.h"
#include "geo.h"

#include "transport_catalogue.h" //���������� ��� domain

namespace renderer {

    struct RenderingStructure {
        double width_;
        double height_;

        double padding_;

        double stop_radius_;
        double line_width_;

        int bus_label_font_size_;
        svg::Point bus_label_offset_;
        int stop_label_font_size_;
        svg::Point stop_label_offset_;

        svg::Color underlayer_color_;
        double underlayer_width_;

        std::vector<svg::Color> color_palette_;
    };

    struct Route {
        std::vector<std::pair<std::string, geo::Coordinates>> stops; //��� + ����������
    };

    inline const double EPSILON = 1e-6;

    class SphereProjector {
    public:
        // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // ���� ����� ����������� ����� �� ������, ��������� ������
            if (points_begin == points_end) {
                return;
            }

            // ������� ����� � ����������� � ������������ ��������
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // ������� ����� � ����������� � ������������ �������
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // ��������� ����������� ��������������� ����� ���������� x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // ��������� ����������� ��������������� ����� ���������� y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // ������������ ��������������� �� ������ � ������ ���������,
                // ���� ����������� �� ���
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *height_zoom;
            }
        }
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        // ���������� ������ � ������� � ���������� ������ SVG-�����������
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {
    public:

        inline void FillRenderingStructure(RenderingStructure& structure) {
            structure_ = structure;
        }

        void OutputSVG(std::ostream& out, const Catalogue::TransportCatalogue& catalogue);

    private:
        void FillRoutes(std::vector<std::pair<std::string, Route>>& route, const Catalogue::TransportCatalogue& catalogue);

        void FillRoutePolylines(std::vector<svg::Polyline>& routes_polylines, const std::vector<std::pair<std::string, Route>>& routes, SphereProjector& proj);
        void FillRouteNames(std::vector<std::pair<std::string, Route>>& routes, const Catalogue::TransportCatalogue& catalogue, std::vector<svg::Text>& routes_names, SphereProjector& proj);
        void FillStopsCircles(std::vector<svg::Circle>& stop_circles, const std::vector<std::pair<std::string, Route>>& routes, SphereProjector& proj);
        void FillStopsNames(std::vector<svg::Text>& stop_names, const std::vector<std::pair<std::string, Route>>& routes, SphereProjector& proj);


        RenderingStructure structure_;
    };

}