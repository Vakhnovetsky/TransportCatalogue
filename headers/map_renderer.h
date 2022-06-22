#pragma once

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cmath>

namespace map_renderer {

    struct MapRenderer {
        double width = 0.0; //ключи, которые задают ширину и высоту в пикселях. 
        double height = 0.0; //ключи, которые задают ширину и высоту в пикселях. 
        double padding = 0.0; //отступ краёв карты от границ SVG-документа.

        double line_width = 0.0; //толщина линий, которыми рисуются автобусные маршруты
        double stop_radius = 0.0; // радиус окружностей, которыми обозначаются остановки

        int bus_label_font_size = 0; //размер текста, которым написаны названия автобусных маршрутов

        //смещение надписи с названием маршрута относительно координат конечной остановки на карте
        double bus_label_offset_dx = 0.0;
        double bus_label_offset_dy = 0.0;

        int stop_label_font_size = 0; //размер текста, которым отображаются названия остановок

        //cмещение названия остановки относительно её координат на карте
        double stop_label_offset_dx = 0.0;
        double stop_label_offset_dy = 0.0;

        ::svg::Color underlayer_color{ "none" }; //цвет подложки под названиями остановок и маршрутов

        //толщина подложки под названиями остановок и маршрутов
        //Задаёт значение атрибута stroke-width элемента <text>
        double underlayer_width = 0.0;

        std::vector<::svg::Color> color_palette; //цветовая палитра. Непустой массив.
    };

    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
            : padding_(padding) {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lng < rhs.lng; });

            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lat < rhs.lat; });

            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;

            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;

            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const {
            return { (coords.lng - min_lon_) * zoom_coeff_ + padding_, (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class DrawRoute {
    public:
        DrawRoute(std::deque<std::pair<::directory::Bus*, std::vector<std::string_view>>>& buses_with_stops, std::set<std::string>& all_stops,
            std::deque<::geo::Coordinates>& coordinates, const map_renderer::MapRenderer& renderer, std::unordered_map<std::string_view,
            ::geo::Coordinates>& stops_with_coordinates);

        void DrawLine(svg::Document& doc);
        void DrawNameRoute(svg::Document& doc);
        void DrawCircleStop(svg::Document& doc);
        void DrawNameStop(svg::Document& doc);

    private:
        std::set<std::string>& all_stops_;
        std::deque<std::pair<::directory::Bus*, std::vector<std::string_view>>>& buses_with_stops_;
        const ::map_renderer::MapRenderer& renderer_;
        SphereProjector sphere_projector_;
        std::unordered_map<std::string_view, ::geo::Coordinates>& stops_with_coordinates_;
    };
}