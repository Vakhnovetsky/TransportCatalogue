#include "transport_router.h"

#include <algorithm>

namespace transport_router {

    using namespace std;

    TransportRouter::TransportRouter(size_t vertex_count)
        : dwg(vertex_count) {
    }

    ::graph::VertexId TransportRouter::GetNewVertexId(std::string_view stop, bool is_transfer) {

        for (const auto& id : id_s_) {
            if (id.name == stop.substr() && id.is_transfer == is_transfer) {
                return id.id;
            }
        }

        TransportRouter::Ids new_id;
        new_id.id = current_id++;
        new_id.is_transfer = is_transfer;
        new_id.name = { stop.begin(), stop.end() };

        id_s_.emplace_back(move(new_id));

        return id_s_.back().id;
    }

    TransportRouter::Ids* TransportRouter::GetStructForName(std::string_view stop, bool is_transfer) {
        for (auto iter = id_s_.begin(); iter != id_s_.end(); ++iter) {
            if ((*iter).name == stop.substr() && (*iter).is_transfer == is_transfer) {
                return &(*iter);
            }
        }
        return nullptr;
    }

    void TransportRouter::WriteNewEdge(::graph::EdgeId edge_id, EdgeInfo info) {
        edges_id_[edge_id] = info;
    }

    TransportRouter::EdgeInfo* TransportRouter::GetVertexForEdge(::graph::EdgeId edge_id) {
        return &edges_id_.at(edge_id);
    }

    ::graph::DirectedWeightedGraph<double>& TransportRouter::CreateGraph(::directory::TransportCatalogue& tr, const std::pair<int, double>& routing_settings) {
        vector<::directory::Bus*> buses = tr.GetBuses();

        for (::directory::Bus* bus : buses) {
            vector<string_view> stops = tr.GetStopsForBus(bus->bus_name);

            for (auto iter_from = stops.begin(); iter_from != stops.end(); ++iter_from) {
                FillInfo(iter_from + 1, stops.end(), 0, tr, routing_settings, 0, bus, GetNewVertexId(*iter_from, false));

                //Ребро одижания автобуса wait
                auto id_from_wait = GetNewVertexId(*iter_from, true);
                auto id_to_wait = GetNewVertexId(*iter_from, false);
                auto edge_wait = dwg.AddEdge({ id_from_wait, id_to_wait,  static_cast<double>(routing_settings.first) });

                ::transport_router::TransportRouter::EdgeInfo info_wait;
                info_wait.id_from = id_from_wait;
                info_wait.id_to = id_to_wait;
                info_wait.time = static_cast<double>(routing_settings.first);
                info_wait.is_bus_type = false;
                info_wait.stop_name = *iter_from;
                WriteNewEdge(edge_wait, info_wait);
            }

            if (!bus->is_roundtrip) {
                //Маршрут не круговой, поэтому надо ехать обратно...
                for (auto iter_from = stops.rbegin(); iter_from != stops.rend(); ++iter_from) {
                    FillInfo(iter_from + 1, stops.rend(), 0, tr, routing_settings, 0, bus, GetNewVertexId(*iter_from, false));
                }
            }
            else {
                //круговой
                auto iter_from = stops.end() - 1;
                auto id_from = GetNewVertexId(*iter_from, false);
                int span_count = 0;

                uint64_t dis = tr.GetDistanceBetweenStops(*iter_from, *stops.begin());
                //Ребро bus
                auto id_to = GetNewVertexId(*stops.begin(), true);

                //Метры переводим в км и часы в минуты
                double time = (dis / 1000.0) * 60 / routing_settings.second;

                auto edge_bus = dwg.AddEdge({ id_from, id_to, time });

                ::transport_router::TransportRouter::EdgeInfo info_bus;
                info_bus.id_from = id_from;
                info_bus.id_to = id_to;
                info_bus.time = time;
                info_bus.span_count = ++span_count;
                info_bus.bus = bus->bus_name;
                info_bus.is_bus_type = true;

                WriteNewEdge(edge_bus, info_bus);

                FillInfo(stops.begin() + 1, iter_from, dis, tr, routing_settings, span_count, bus, id_from);
            }
        }
        return dwg;
    }

    ::graph::DirectedWeightedGraph<double>& TransportRouter::Restore(std::vector <::graph::Edge<double>>& edges, ::graph::VertexId& curr_id,
        std::deque<Ids>& id_s, std::map<::graph::EdgeId, EdgeInfo>& edges_id) {

        current_id = curr_id;
        id_s_ = id_s;
        edges_id_ = edges_id;

        for (const auto& edge : edges) {
            dwg.AddEdge(edge);
        }

        return dwg;
    }

    ::graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() {
        return dwg;
    }

    TransportRouter::RouteInfo TransportRouter::GetRoute(::graph::Router<double>& router, std::string_view from, std::string_view to) {

        deque<TransportRouter::EdgeInfo*> result;

        if (GetStructForName(from, true) != nullptr && GetStructForName(to, true) != nullptr) {
            auto built_route = router.BuildRoute(GetStructForName(from, true)->id, GetStructForName(to, true)->id);

            if (built_route.has_value()) {

                for (auto edge : built_route.value().edges) {
                    result.push_back(GetVertexForEdge(edge));
                }

                return { result, built_route.value().weight };
            }
        }
        return { result, -1 };
    }

    void TransportRouter::GetVariable(::graph::VertexId& curr_id, std::deque<Ids>& id_s, std::map<::graph::EdgeId, EdgeInfo>& edges_id) {
        curr_id = current_id;
        id_s = id_s_;
        edges_id = edges_id_;
    }
} //namespace transport_router