#include "domain.h"
#include "graph.h"
#include "json_reader.h"
#include "router.h"
#include "serialization.h"
#include "transport_catalogue.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        ::directory::TransportCatalogue tr;
        ::serialization_space::SerializeVariable serialize_variable;
        std::string path;

        ::directory::json_detail::JsonReader j_reader;
        j_reader.ReadMakeBase(std::cin, serialize_variable, path);

        ::renderer::RequestHandler rh(tr, serialize_variable.renderer, serialize_variable.routing_settings);
        rh.FillTransportCatalogue(serialize_variable);
        rh.SetRouterWithNewGraph();
        rh.GetVariableForGraph(serialize_variable.edges, serialize_variable.vertex_count);
        rh.GetVariableTransportRouter(serialize_variable.current_id, serialize_variable.id_s_, serialize_variable.edges_id_);

        ::serialization_space::Serialization srlz(serialize_variable, path);
        srlz.Serialize();
    }
    else if (mode == "process_requests"sv) {
        std::vector<::directory::json_detail::QueryStat> stat_queries;
        ::directory::TransportCatalogue tr;
        ::serialization_space::SerializeVariable serialize_variable;
        std::string path;

        ::directory::json_detail::JsonReader j_reader;
        j_reader.ReadProcessRequests(std::cin, stat_queries, path);
        ::serialization_space::Serialization srlz(serialize_variable, path);
        srlz.Deserialize();

        ::renderer::RequestHandler rh(tr, serialize_variable.renderer, serialize_variable.routing_settings);
        rh.FillTransportCatalogue(serialize_variable);
        rh.RestoreGraph(serialize_variable.edges, serialize_variable.vertex_count, serialize_variable.current_id, serialize_variable.id_s_, serialize_variable.edges_id_);
        j_reader.PrintStatRequests(rh, stat_queries, std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}



