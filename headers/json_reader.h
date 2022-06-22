#pragma once

#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <deque>
#include <sstream>
#include <vector>

namespace directory {
    namespace json_detail {
        class JsonReader final {
        public:
            void ReadMakeBase(std::istream& is, ::serialization_space::SerializeVariable& serialize_variable, std::string& path);

            void ReadProcessRequests(std::istream& is, std::vector<QueryStat>& stat_queries, std::string& path);

            void PrintStatRequests(::renderer::RequestHandler& rh, std::vector<QueryStat>& queries, std::ostream& os);

        private:
            void ReadBaseRequests(const ::json::Node& request, std::vector<QueryBus>& bus_queries, std::vector<QueryStop>& stop_queries);
            void ReadRenderSettings(const ::json::Node& request, ::map_renderer::MapRenderer& renderer);
            void ReadStatRequests(const ::json::Node& request, std::vector<QueryStat>& stat_queries);
            void ReadRoutingSettings(const ::json::Node& request, std::pair<int, double>& routing_settings);
            void ReadSerializationSettings(const ::json::Node& request, std::string& path);

            void PrintBus(Bus* bus, int id, ::json::Array& out_array);
            void PrintStop(bool contain_stop, std::set<std::string_view>& buses, int id, ::json::Array& out_array);
            void PrintMap(svg::Document doc, int id, ::json::Array& out_array);
            void PrintRoute(const QueryStat& query_out, ::json::Array& out_array, ::renderer::RequestHandler& rh);
            void PrintNotFoundRoute(const QueryStat& query_out, ::json::Array& out_array);

            ::svg::Color GetColor(const ::json::Node& node);
        };
    }
}