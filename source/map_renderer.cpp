#include "map_renderer.h"

namespace map_renderer {

	using namespace std;

	DrawRoute::DrawRoute(deque<pair<::directory::Bus*, vector<string_view>>>& buses_with_stops, set<string>& all_stops, deque<::geo::Coordinates>& coordinates,
						 const map_renderer::MapRenderer& renderer, unordered_map<string_view, ::geo::Coordinates>& stops_with_coordinates)
		: all_stops_(all_stops)
		, buses_with_stops_(buses_with_stops)
		, renderer_(renderer)
		, sphere_projector_(coordinates.begin(), coordinates.end(), renderer_.width, renderer_.height, renderer_.padding) 
		, stops_with_coordinates_(stops_with_coordinates) {
	}

	//Отрисовка линий
	void DrawRoute::DrawLine(svg::Document& doc) {

		int nubmer_color_palette = 0;

		for (auto& [bus, stops]: buses_with_stops_) {
			
			if (stops.size() > 0) {
				::svg::Polyline polyline;

				for (const auto& stop : stops) {
					polyline.AddPoint(sphere_projector_(stops_with_coordinates_.at(stop)));
				}

				if (!bus->is_roundtrip) {
					for (auto iter = stops.rbegin() + 1; iter != stops.rend(); ++iter) {
						polyline.AddPoint(sphere_projector_(stops_with_coordinates_.at(*iter)));
					}
				}

				polyline
					.SetFillColor("none")
					.SetStrokeColor(renderer_.color_palette[nubmer_color_palette])
					.SetStrokeWidth(renderer_.line_width)
					.SetStrokeLineJoin(::svg::StrokeLineJoin::ROUND)
					.SetStrokeLineCap(::svg::StrokeLineCap::ROUND);

				doc.Add(polyline);

				if (++nubmer_color_palette == static_cast<int>(renderer_.color_palette.size())) {
					nubmer_color_palette = 0;
				}
			}
		}
	}

	//Названия маршрутов
	void DrawRoute::DrawNameRoute(svg::Document& doc) {

		int nubmer_color_palette = 0;

		for (auto& [bus, stops] : buses_with_stops_) {
			if (stops.size() > 0) {
				::svg::Text name_bus, name_bus_ground;
				name_bus
					.SetFillColor(renderer_.color_palette[nubmer_color_palette])
					.SetPosition(sphere_projector_(stops_with_coordinates_.at(*stops.begin())))
					.SetOffset({ renderer_.bus_label_offset_dx, renderer_.bus_label_offset_dy })
					.SetFontSize(renderer_.bus_label_font_size)
					.SetFontFamily("Verdana")
					.SetFontWeight("bold")
					.SetData(bus->bus_name);

				name_bus_ground
					.SetFillColor(renderer_.underlayer_color)
					.SetStrokeColor(renderer_.underlayer_color)
					.SetStrokeWidth(renderer_.underlayer_width)
					.SetStrokeLineCap(::svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(::svg::StrokeLineJoin::ROUND)
					.SetPosition(sphere_projector_(stops_with_coordinates_.at(*stops.begin())))
					.SetOffset({ renderer_.bus_label_offset_dx, renderer_.bus_label_offset_dy })
					.SetFontSize(renderer_.bus_label_font_size)
					.SetFontFamily("Verdana")
					.SetFontWeight("bold")
					.SetData(bus->bus_name);


				doc.Add(name_bus_ground);
				doc.Add(name_bus);			
				
				if (!bus->is_roundtrip && *stops.begin() != *(stops.end() - 1)) {
					name_bus.SetPosition(sphere_projector_(stops_with_coordinates_.at(*(stops.end() - 1))));
					name_bus_ground.SetPosition(sphere_projector_(stops_with_coordinates_.at(*(stops.end() - 1))));

					doc.Add(name_bus_ground);
					doc.Add(name_bus);
				}

				if (++nubmer_color_palette == static_cast<int>(renderer_.color_palette.size())) {
					nubmer_color_palette = 0;
				}
			}
		}
	}

	//Символы остановок
	void DrawRoute::DrawCircleStop(svg::Document& doc) {
		
		for (const auto& stop : all_stops_) {
			::svg::Circle circle;
			circle
				.SetCenter(sphere_projector_(stops_with_coordinates_.at(stop)))
				.SetRadius(renderer_.stop_radius)
				.SetFillColor("white");
			doc.Add(circle);
		}
	}

	//Названия остановок
	void DrawRoute::DrawNameStop(svg::Document& doc) {
		
		for (const auto& stop : all_stops_) {
			::svg::Text name_stop, name_stop_ground;

			name_stop
				.SetFillColor("black")

				.SetPosition(sphere_projector_(stops_with_coordinates_.at(stop)))
				.SetOffset({ renderer_.stop_label_offset_dx, renderer_.stop_label_offset_dy })
				.SetFontSize(renderer_.stop_label_font_size)
				.SetFontFamily("Verdana")
				.SetData(stop);

			name_stop_ground
				.SetFillColor(renderer_.underlayer_color)
				.SetStrokeColor(renderer_.underlayer_color)
				.SetStrokeWidth(renderer_.underlayer_width)
				.SetStrokeLineCap(::svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(::svg::StrokeLineJoin::ROUND)

				.SetPosition(sphere_projector_(stops_with_coordinates_.at(stop)))
				.SetOffset({ renderer_.stop_label_offset_dx, renderer_.stop_label_offset_dy })
				.SetFontSize(renderer_.stop_label_font_size)
				.SetFontFamily("Verdana")
				.SetData(stop);

			doc.Add(name_stop_ground);
			doc.Add(name_stop);
		}
	}
}