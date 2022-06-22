#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <deque>
#include <map>

namespace transport_router {
    class TransportRouter {
    public:
        struct Ids {
            ::graph::VertexId id;
            bool is_transfer;
            std::string name;
        };

        struct EdgeInfo {
            ::graph::VertexId id_from;
            ::graph::VertexId id_to;

            std::string bus;
            int span_count;
            double time;
            bool is_bus_type;
            std::string stop_name;
        };

        struct RouteInfo {
            std::deque<EdgeInfo*> edges;
            double total_time;
        };

        TransportRouter(size_t vertex_count);
        ::graph::DirectedWeightedGraph<double>& CreateGraph(::directory::TransportCatalogue& tr, const std::pair<int, double>& routing_settings);
        ::graph::DirectedWeightedGraph<double>& GetGraph();
        RouteInfo GetRoute(::graph::Router<double>& router, std::string_view from, std::string_view to);

        ::graph::DirectedWeightedGraph<double>& Restore(std::vector <::graph::Edge<double>>& edges, ::graph::VertexId& curr_id,
            std::deque<Ids>& id_s, std::map<::graph::EdgeId, EdgeInfo>& edges_id);

        void GetVariable(::graph::VertexId& curr_id, std::deque<Ids>& id_s, std::map<::graph::EdgeId, EdgeInfo>& edges_id);

    private:
        ::graph::VertexId current_id = 0;
        std::deque<Ids> id_s_;
        std::map<::graph::EdgeId, EdgeInfo> edges_id_;

        ::graph::DirectedWeightedGraph<double> dwg;

        ::graph::VertexId GetNewVertexId(std::string_view stop, bool is_transfer);
        Ids* GetStructForName(std::string_view stop, bool is_transfer);
        void WriteNewEdge(::graph::EdgeId edge_id, EdgeInfo info);
        EdgeInfo* GetVertexForEdge(::graph::EdgeId edge_id);

        template<typename Iterator>
        void FillInfo(Iterator iter_to, Iterator iter_from, uint64_t dis, ::directory::TransportCatalogue& tr, const std::pair<int, double>& routing_settings, 
                      int span_count, ::directory::Bus* bus, ::graph::VertexId id_from) {

            using namespace std::literals;

            for (; iter_to != iter_from; ++iter_to) {
                dis += tr.GetDistanceBetweenStops(*(iter_to - 1), *iter_to);
                auto id_to = GetNewVertexId(*iter_to, true);
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
            }
        }
    };
} //namespace transport_router