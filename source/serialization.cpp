#include "serialization.h"
#include "svg.h"

#include <fstream>
#include <variant>

namespace serialization_space {
	using namespace std::literals;

	Serialization::Serialization(SerializeVariable& sv, std::string path)
		:sv_(sv)
		, path_(path) {}

	void Serialization::Serialize() {
		if (tr_proto_.has_value()) {
			tr_proto_.reset();
		}
		tr_proto_ = std::make_optional<::transport_catalogue_serialize::TransportCatalogue>();

		SerializeTransportRouter();
		SerializeStopQueries();
		SerializeBusQueries();
		SerializeRoutingSettings();
		SerializeGraph();
		SerializeMapRenderer();

		std::ofstream out_file(path_, std::ios::binary);
		tr_proto_.value().SerializeToOstream(&out_file);
	}

	void Serialization::SerializeStopQueries() {
		for (const ::directory::json_detail::QueryStop& stop : sv_.stop_queries) {
			::transport_catalogue_serialize::Stop new_stop;

			new_stop.set_name(stop.stop);
			new_stop.set_latitude(stop.coordinates.lat);
			new_stop.set_longitude(stop.coordinates.lng);

			for (const auto& [name, distance] : stop.distance_to_stop) {
				(*new_stop.mutable_road_distances())[name] = static_cast<int32_t>(distance);
			}

			tr_proto_.value().mutable_stops()->Add(std::move(new_stop));
		}
	}

	void Serialization::SerializeBusQueries() {
		for (const ::directory::json_detail::QueryBus& bus : sv_.bus_queries) {
			::transport_catalogue_serialize::Bus new_bus;

			new_bus.set_is_roundtrip(bus.is_roundtrip);
			new_bus.set_name(bus.bus);

			for (const int32_t& id : bus_edge_ids_.at(bus.bus)) {
				new_bus.add_ids(id);
			}

			for (const std::string& stop : bus.stops) {
				new_bus.add_stops(stop);
			}

			tr_proto_.value().mutable_buses()->Add(std::move(new_bus));
		}
	}

	void Serialization::SerializeRoutingSettings() {
		::graph_proto::RoutingSettings routing_s;
		routing_s.set_bus_wait_time(sv_.routing_settings.first);
		routing_s.set_bus_velocity(sv_.routing_settings.second);
		*tr_proto_.value().mutable_rout_s() = std::move(routing_s);
	}

	void Serialization::SerializeMapRenderer() {
		::map_renderer_proto::RenderSettings render_s;
		render_s.set_width(sv_.renderer.width);
		render_s.set_height(sv_.renderer.height);
		render_s.set_padding(sv_.renderer.padding);
		render_s.set_line_width(sv_.renderer.line_width);
		render_s.set_stop_radius(sv_.renderer.stop_radius);
		render_s.set_bus_label_font_size(sv_.renderer.bus_label_font_size);
		render_s.set_bus_label_offset_dx(sv_.renderer.bus_label_offset_dx);
		render_s.set_bus_label_offset_dy(sv_.renderer.bus_label_offset_dy);
		render_s.set_stop_label_font_size(sv_.renderer.stop_label_font_size);
		render_s.set_stop_label_offset_dx(sv_.renderer.stop_label_offset_dx);
		render_s.set_stop_label_offset_dy(sv_.renderer.stop_label_offset_dy);
		render_s.set_underlayer_width(sv_.renderer.underlayer_width);

		*render_s.mutable_underlayer_color() = std::move(GetColorProto(sv_.renderer.underlayer_color));

		for (const ::svg::Color& c : sv_.renderer.color_palette) {
			render_s.mutable_color_palette()->Add(GetColorProto(c));
		}

		*tr_proto_.value().mutable_rend_s() = std::move(render_s);
	}

	void Serialization::SerializeGraph() {
		::graph_proto::Edges edges_s;

		for (const auto& elem : sv_.edges) {
			::graph_proto::Edge edge_s;
			edge_s.set_from(static_cast<int32_t>(elem.from));
			edge_s.set_to(static_cast<int32_t>(elem.to));
			edge_s.set_weight(elem.weight);
			edges_s.mutable_edges()->Add(std::move(edge_s));
		}

		edges_s.set_vertex_count(static_cast<int32_t>(sv_.vertex_count));
		*tr_proto_.value().mutable_edges() = std::move(edges_s);
	}

	void Serialization::SerializeTransportRouter() {
		::transport_router_serialize::TransportRouter transport_router;

		transport_router.set_current_id(static_cast<int32_t>(sv_.current_id));

		::transport_router_serialize::Ids ids;
		for (const auto& elem : sv_.id_s_) {
			::transport_router_serialize::Ids ids;
			ids.set_id(static_cast<int32_t>(elem.id));
			ids.set_name(elem.name);
			ids.set_is_transfer(elem.is_transfer);
			transport_router.mutable_ids()->Add(std::move(ids));
		}

		for (const auto& [id, edge_info] : sv_.edges_id_) {
			::transport_router_serialize::EdgeInfo new_edge_info;

			new_edge_info.set_id_from(static_cast<int32_t>(edge_info.id_from));
			new_edge_info.set_id_to(static_cast<int32_t>(edge_info.id_to));
			new_edge_info.set_span_count(edge_info.span_count);
			new_edge_info.set_time(edge_info.time);
			new_edge_info.set_is_bus_type(edge_info.is_bus_type);
			new_edge_info.set_stop_name(edge_info.stop_name);

			if (edge_info.bus.size() > 0) {
				bus_edge_ids_[edge_info.bus].insert(static_cast<int32_t>(id));
			}

			(*transport_router.mutable_edges_id_())[static_cast<int32_t>(id)] = new_edge_info;
		}

		*tr_proto_.value().mutable_tr() = std::move(transport_router);
	}

	::svg_proto::Color Serialization::GetColorProto(const ::svg::Color& color) {
		::svg_proto::Color color_proto;
		if (std::holds_alternative<std::string>(color)) {
			color_proto.set_c_string(std::get<std::string>(color));
		}
		else if (std::holds_alternative<svg::Rgb>(color)) {
			svg::Rgb rgb = std::get<svg::Rgb>(color);

			::svg_proto::Rgb new_rgb;
			new_rgb.set_red(rgb.red);
			new_rgb.set_green(rgb.green);
			new_rgb.set_blue(rgb.blue);

			*color_proto.mutable_c_rgb() = std::move(new_rgb);
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			svg::Rgba rgba = std::get<svg::Rgba>(color);

			::svg_proto::Rgba new_rgba;
			new_rgba.set_red(rgba.red);
			new_rgba.set_green(rgba.green);
			new_rgba.set_blue(rgba.blue);
			new_rgba.set_opacity(rgba.opacity);

			*color_proto.mutable_c_rgba() = std::move(new_rgba);
		}

		return color_proto;
	}

	void Serialization::Deserialize() {
		if (tr_proto_.has_value()) {
			tr_proto_.reset();
		}
		tr_proto_ = std::make_optional<::transport_catalogue_serialize::TransportCatalogue>();

		std::ifstream in_file(path_, std::ios::binary);

		if (tr_proto_.value().ParseFromIstream(&in_file)) {
			DeserializeBusQueries();
			DeserializeStopQueries();
			DeserializeRoutingSettings();
			DeserializeGraph();
			DeserializeMapRenderer();
			DeserializeTransportRouter();
		}
	}

	void Serialization::DeserializeStopQueries() {
		for (const auto& stop : *tr_proto_.value().mutable_stops()) {
			::directory::json_detail::QueryStop new_stop;
			new_stop.stop = stop.name();
			new_stop.coordinates = { stop.latitude(), stop.longitude() };
			for (const auto& [name, distance] : stop.road_distances()) {
				new_stop.distance_to_stop[name] = distance;
			}
			sv_.stop_queries.emplace_back(std::move(new_stop));
		}
	}

	void Serialization::DeserializeBusQueries() {
		for (const auto& bus : *tr_proto_.value().mutable_buses()) {
			::directory::json_detail::QueryBus new_bus;
			new_bus.bus = bus.name();
			new_bus.is_roundtrip = bus.is_roundtrip();
			for (const auto& stop : bus.stops()) {
				new_bus.stops.push_back(stop);
			}

			for (const auto& id : bus.ids()) {
				bus_edge_ids_[bus.name()].insert(id);
			}
			
			sv_.bus_queries.emplace_back(std::move(new_bus));
		}
	}

	void Serialization::DeserializeRoutingSettings() {
		::graph_proto::RoutingSettings routing_s = tr_proto_.value().rout_s();
		sv_.routing_settings = std::make_pair(routing_s.bus_wait_time(), routing_s.bus_velocity());
	}

	void Serialization::DeserializeMapRenderer() {
		::map_renderer_proto::RenderSettings render_s = tr_proto_.value().rend_s();
		sv_.renderer.width = render_s.width();
		sv_.renderer.height = render_s.height();
		sv_.renderer.padding = render_s.padding();
		sv_.renderer.line_width = render_s.line_width();
		sv_.renderer.stop_radius = render_s.stop_radius();
		sv_.renderer.bus_label_font_size = render_s.bus_label_font_size();
		sv_.renderer.bus_label_offset_dx = render_s.bus_label_offset_dx();
		sv_.renderer.bus_label_offset_dy = render_s.bus_label_offset_dy();
		sv_.renderer.stop_label_font_size = render_s.stop_label_font_size();
		sv_.renderer.stop_label_offset_dx = render_s.stop_label_offset_dx();
		sv_.renderer.stop_label_offset_dy = render_s.stop_label_offset_dy();
		sv_.renderer.underlayer_width = render_s.underlayer_width();
		sv_.renderer.underlayer_color = GetColorSvg(render_s.underlayer_color());

		for (::svg_proto::Color color : render_s.color_palette()) {
			sv_.renderer.color_palette.push_back(GetColorSvg(color));
		}
	}

	void Serialization::DeserializeGraph() {
		::graph_proto::Edges edges_s = tr_proto_.value().edges();

		sv_.vertex_count = edges_s.vertex_count();

		for (const ::graph_proto::Edge& edge : edges_s.edges()) {
			::graph::Edge<double> e;
			e.from = static_cast<::graph::VertexId>(edge.from());
			e.to = static_cast<::graph::VertexId>(edge.to());
			e.weight = edge.weight();
			sv_.edges.emplace_back(std::move(e));
		}
	}

	void Serialization::DeserializeTransportRouter() {
		
		::transport_router_serialize::TransportRouter transport_router = tr_proto_.value().tr();

		sv_.current_id = transport_router.current_id();

		for (const ::transport_router_serialize::Ids& id : transport_router.ids()) {
			::transport_router::TransportRouter::Ids new_id;
			new_id.id = id.id();
			new_id.is_transfer = id.is_transfer();
			new_id.name = id.name();
			sv_.id_s_.emplace_back(std::move(new_id));
		}

		for (const auto& [id, edge_info] : transport_router.edges_id_()) {

			::transport_router::TransportRouter::EdgeInfo new_edge_info;

			new_edge_info.id_from = edge_info.id_from();
			new_edge_info.id_to = edge_info.id_to();
			new_edge_info.bus = FindBusName(id);
			new_edge_info.span_count = edge_info.span_count();
			new_edge_info.time = edge_info.time();
			new_edge_info.is_bus_type = edge_info.is_bus_type();
			new_edge_info.stop_name = edge_info.stop_name();

			sv_.edges_id_.emplace(id, std::move(new_edge_info));
		}
		
	}

	std::string Serialization::FindBusName(int32_t id) {
		for (const auto& [name, ids] : bus_edge_ids_) {
			if (ids.find(id) != ids.end()) {
				return name;
			}
		}
		return ""s;
	}

	::svg::Color Serialization::GetColorSvg(::svg_proto::Color color) {
		::svg::Color color_svg;

		if (!color.c_string().empty()) {
			color_svg = color.c_string();
		}
		else if (color.has_c_rgb()) {
			::svg_proto::Rgb rgb = color.c_rgb();
			color_svg = ::svg::Rgb{ static_cast<uint8_t>(rgb.red()), static_cast<uint8_t>(rgb.green()), static_cast<uint8_t>(rgb.blue()) };
		}
		else if (color.has_c_rgba()) {
			::svg_proto::Rgba rgba = color.c_rgba();
			color_svg = ::svg::Rgba{ static_cast<uint8_t>(rgba.red()), static_cast<uint8_t>(rgba.green()), static_cast<uint8_t>(rgba.blue()), rgba.opacity() };
		}

		return color_svg;
	}
}