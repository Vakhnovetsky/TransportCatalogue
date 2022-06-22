#include "request_handler.h"

using namespace std;

namespace renderer {
	RequestHandler::RequestHandler(::directory::TransportCatalogue& db, const map_renderer::MapRenderer& renderer, const pair<int, double>& routing_settings)
		: db_(db)
		, renderer_(renderer)
		, routing_settings_(routing_settings)
	{
	}

	void RequestHandler::RestoreGraph(std::vector <::graph::Edge<double>>& edges, ::graph::VertexId& vertex_count, ::graph::VertexId& current_id,
		std::deque<transport_router::TransportRouter::Ids>& id_s_, std::map<::graph::EdgeId, transport_router::TransportRouter::EdgeInfo>& edges_id_) {
		tr_rout_ = std::make_optional<::transport_router::TransportRouter>(vertex_count);
		router_.emplace(tr_rout_.value().Restore(edges, current_id, id_s_, edges_id_));
	}

	void RequestHandler::SetRouterWithNewGraph() {
		tr_rout_ = std::make_optional<::transport_router::TransportRouter>(db_.GetCountStops() * 2);
		router_.emplace(tr_rout_.value().CreateGraph(db_, routing_settings_));
	}

	::directory::Bus* RequestHandler::GetInfoAboutRoute(const string_view& bus_name) {
		return db_.GetInfoAboutRoute(bus_name);
	}

	tuple<bool, set<string_view>> RequestHandler::GetBusesForStop(const string_view& stop_name) {
		return db_.GetBusesForStop(stop_name);
	}

	svg::Document RequestHandler::RenderMap() const {
		svg::Document result;

		unordered_map<string_view, ::geo::Coordinates> stops_with_coordinates = db_.GetStopsWithCoordinates();
		deque<::geo::Coordinates> coordinates;
		for (const auto& [stop, coordinate]: stops_with_coordinates) {
			coordinates.push_back(coordinate);
		}

		//Все остановки со всех маршрутов
		set<string> all_stops;

		vector<::directory::Bus*> buses = db_.GetBuses();

		deque<pair<::directory::Bus*, vector<string_view>>> buses_with_stops;
		
		for (const auto& bus : buses) {
			vector<string_view> stops = db_.GetStopsForBus(bus->bus_name);

			if (stops.size() > 0) {
				all_stops.insert(stops.begin(), stops.end());
				buses_with_stops.push_back({ bus , move(stops) });
			}
		}

		::map_renderer::DrawRoute dr(buses_with_stops, all_stops, coordinates, renderer_, stops_with_coordinates);

		//Отрисовка линий
		dr.DrawLine(result);

		//Названия маршрутов
		dr.DrawNameRoute(result);

		//Символы остановок
		dr.DrawCircleStop(result);

		//Названия остановок
		dr.DrawNameStop(result);

		return result;
	}

	::transport_router::TransportRouter::RouteInfo RequestHandler::GetRouteForQuery(const ::directory::json_detail::QueryStat& query) {
		return tr_rout_.value().GetRoute(router_.value(), query.from, query.to);
	}

	void RequestHandler::GetVariableForGraph(std::vector <::graph::Edge<double>>& edges, ::graph::VertexId& vertex_count) {
		vertex_count = tr_rout_.value().GetGraph().GetVertexCount();
		edges = std::move(tr_rout_.value().GetGraph().GetEdges());
	}

	void RequestHandler::GetVariableTransportRouter(::graph::VertexId& current_id, std::deque<transport_router::TransportRouter::Ids>& id_s_, std::map<::graph::EdgeId, transport_router::TransportRouter::EdgeInfo>& edges_id_) {
		tr_rout_.value().GetVariable(current_id, id_s_, edges_id_);
	}

	void RequestHandler::FillTransportCatalogue(::serialization_space::SerializeVariable& sv) {
		for (const auto& query : sv.stop_queries) {
			db_.AddStation(query.stop, query.coordinates);
		}

		for (auto& query : sv.stop_queries) {
			db_.AddStationDistance(query.stop, query.distance_to_stop);
		}

		for (auto& query : sv.bus_queries) {
			db_.AddRoute(query.bus, query.stops, query.is_roundtrip);
		}
	}
}