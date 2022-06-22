#pragma once

#include "geo.h"
#include "domain.h"
#include "map_renderer.h"
#include "router.h"
#include "serialization.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <deque>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

namespace renderer {

    // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
    // с другими подсистемами приложения.
    // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
    class RequestHandler {
    public:
        RequestHandler(::directory::TransportCatalogue& db, const map_renderer::MapRenderer& renderer, const std::pair<int, double>& routing_settings);

        ::directory::Bus* GetInfoAboutRoute(const std::string_view& bus_name);

        std::tuple<bool, std::set<std::string_view>> GetBusesForStop(const std::string_view& stop_name);

        svg::Document RenderMap() const;

        ::transport_router::TransportRouter::RouteInfo GetRouteForQuery(const ::directory::json_detail::QueryStat& query);

        //void SetTransportRouter();

        //void CreateNewGraph();
        /*
        ::graph::VertexId current_id = 0;
        std::deque<Ids> id_s_;
        std::map<::graph::EdgeId, EdgeInfo> edges_id_;
        */

        void RestoreGraph(std::vector <::graph::Edge<double>>& edges, ::graph::VertexId& vertex_count, ::graph::VertexId& current_id, 
            std::deque<transport_router::TransportRouter::Ids>& id_s_, std::map<::graph::EdgeId, transport_router::TransportRouter::EdgeInfo>& edges_id_);

        void SetRouterWithNewGraph();

        void GetVariableForGraph(std::vector <::graph::Edge<double>>& edges, ::graph::VertexId& vertex_count);
        void GetVariableTransportRouter(::graph::VertexId& current_id, std::deque<transport_router::TransportRouter::Ids>& id_s_, std::map<::graph::EdgeId, transport_router::TransportRouter::EdgeInfo>& edges_id_);

        void FillTransportCatalogue(::serialization_space::SerializeVariable& sv);

    private:
        ::directory::TransportCatalogue& db_;
        const map_renderer::MapRenderer& renderer_;
        const std::pair<int, double>& routing_settings_;
        std::optional<::transport_router::TransportRouter> tr_rout_;
        //::transport_router::TransportRouter tr_rout_;
        std::optional<::graph::Router<double>> router_;
    };
}

//pair<deque<::transport_router::TransportRouter::EdgeInfo*>, double> route = tr_rout.GetRoute(router, query_out.from, query_out.to);