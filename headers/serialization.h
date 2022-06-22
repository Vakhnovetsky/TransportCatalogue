#pragma once

#include "domain.h"
#include "graph.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include "graph.pb.h"
#include "map_renderer.pb.h"
#include "svg.pb.h"
#include "transport_catalogue.pb.h"
#include "transport_router.pb.h"

#include <string>
#include <vector>

namespace serialization_space {
    struct SerializeVariable{
        std::vector<::directory::json_detail::QueryBus> bus_queries;
        std::vector<::directory::json_detail::QueryStop> stop_queries;
        std::pair<int, double> routing_settings;
        ::map_renderer::MapRenderer renderer;
        std::vector<::graph::Edge<double>> edges;
        ::graph::VertexId vertex_count;

        ::graph::VertexId current_id = 0;
        std::deque<::transport_router::TransportRouter::Ids> id_s_;
        std::map<::graph::EdgeId, ::transport_router::TransportRouter::EdgeInfo> edges_id_;
    };

    class Serialization {
    public:
        Serialization(SerializeVariable& sv, std::string path);
        void Serialize();
        void Deserialize();

    private:
        std::string path_;
        SerializeVariable& sv_;
        std::optional<::transport_catalogue_serialize::TransportCatalogue> tr_proto_;

        ::svg_proto::Color GetColorProto(const ::svg::Color& color);
        ::svg::Color GetColorSvg(::svg_proto::Color color);

        std::map <std::string, std::set<int32_t>> bus_edge_ids_;
        std::string FindBusName(int32_t id);

        void SerializeStopQueries();
        void SerializeBusQueries();
        void SerializeRoutingSettings();
        void SerializeGraph();
        void SerializeMapRenderer();
        void SerializeTransportRouter();

        void DeserializeStopQueries();
        void DeserializeBusQueries();
        void DeserializeRoutingSettings();
        void DeserializeGraph();
        void DeserializeMapRenderer();
        void DeserializeTransportRouter();
    };
}