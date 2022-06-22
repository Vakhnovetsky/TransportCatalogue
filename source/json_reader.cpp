#include "map_renderer.h"
#include "json_reader.h"

namespace directory {
    namespace json_detail {
        using namespace std;

        void JsonReader::ReadBaseRequests(const ::json::Node& request, vector<QueryBus>& bus_queries, vector<QueryStop>& stop_queries) {
            for (const auto& arr : request.AsArray()) {
                const auto& req = arr.AsDict();

                const string& base_requests_type = req.at("type"s).AsString();

                if (base_requests_type == "Bus"s) {
                    QueryBus query;
                    query.bus = req.at("name"s).AsString();

                    for (const auto& stop : req.at("stops"s).AsArray()) {
                        query.stops.emplace_back(stop.AsString());
                    }

                    query.is_roundtrip = req.at("is_roundtrip"s).AsBool();

                    bus_queries.emplace_back(move(query));
                }
                else if (base_requests_type == "Stop"s) {
                    QueryStop query;

                    query.stop = req.at("name"s).AsString();
                    query.coordinates.lat = req.at("latitude"s).AsDouble();
                    query.coordinates.lng = req.at("longitude"s).AsDouble();

                    for (const auto& [stop, distances] : req.at("road_distances"s).AsDict()) {
                        query.distance_to_stop[stop] = distances.AsInt();
                    }

                    stop_queries.emplace_back(move(query));
                }
            }
        }

        void JsonReader::ReadRenderSettings(const ::json::Node& request, ::map_renderer::MapRenderer& renderer) {
            const auto& settings = request.AsDict();
            renderer.width = settings.at("width").AsDouble();

            renderer.height = settings.at("height").AsDouble();

            renderer.padding = settings.at("padding").AsDouble();
            renderer.line_width = settings.at("line_width").AsDouble();
            renderer.stop_radius = settings.at("stop_radius").AsDouble();

            renderer.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
            const auto& bus_label_offset = settings.at("bus_label_offset").AsArray();
            renderer.bus_label_offset_dx = bus_label_offset.at(0).AsDouble();
            renderer.bus_label_offset_dy = bus_label_offset.at(1).AsDouble();

            renderer.stop_label_font_size = settings.at("stop_label_font_size").AsInt();
            const auto& stop_label_offset = settings.at("stop_label_offset").AsArray();
            renderer.stop_label_offset_dx = stop_label_offset.at(0).AsDouble();
            renderer.stop_label_offset_dy = stop_label_offset.at(1).AsDouble();

            renderer.underlayer_width = settings.at("underlayer_width").AsDouble();

            renderer.underlayer_color = GetColor(settings.at("underlayer_color"));

            const auto& color_palette = settings.at("color_palette").AsArray();

            for (const auto& color : color_palette) {
                renderer.color_palette.push_back(GetColor(color));
            }
        }

        void JsonReader::ReadStatRequests(const ::json::Node& request, vector<QueryStat>& stat_queries) {
            for (const auto& arr : request.AsArray()) {
                const auto& req = arr.AsDict();
                QueryStat query;

                query.id = req.at("id"s).AsInt();
                query.type = req.at("type"s).AsString();

                if (query.type == "Bus"s || query.type == "Stop"s) {
                    query.name = req.at("name"s).AsString();
                }

                if (query.type == "Route"s) {
                    query.from = req.at("from"s).AsString();
                    query.to = req.at("to"s).AsString();
                }

                stat_queries.emplace_back(move(query));
            }
        }

        void JsonReader::ReadRoutingSettings(const ::json::Node& request, pair<int, double>& routing_settings) {
            const auto& req = request.AsDict();
            routing_settings = { req.at("bus_wait_time"s).AsInt(), req.at("bus_velocity"s).AsDouble() };
        }

        void JsonReader::ReadSerializationSettings(const ::json::Node& request, string& path) {
            path = request.AsDict().at("file"s).AsString();
        }

        void JsonReader::ReadMakeBase(istream& is, ::serialization_space::SerializeVariable& serialize_variable, string& path) {
            try {
                ::json::Document doc = ::json::Load(is);

                const auto& requests = doc.GetRoot().AsDict();
                for (const auto& [req_type, request] : requests) {

                    if (req_type == "base_requests"s) {
                        ReadBaseRequests(request, serialize_variable.bus_queries, serialize_variable.stop_queries);
                    }
                    else if (req_type == "render_settings"s) {
                        ReadRenderSettings(request, serialize_variable.renderer);
                    }
                    else if (req_type == "routing_settings"s) {
                        ReadRoutingSettings(request, serialize_variable.routing_settings);
                    }
                    else if (req_type == "serialization_settings"s) {
                        ReadSerializationSettings(request, path);
                    }
                }

                
            }
            catch (const std::exception& e) {
                std::cerr << "exception thrown: "s << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unexpected error"s << std::endl;
            }
        }

        void JsonReader::ReadProcessRequests(std::istream& is, vector<QueryStat>& stat_queries, string& path) {
            try {
                ::json::Document doc = ::json::Load(is);

                const auto& requests = doc.GetRoot().AsDict();
                for (const auto& [req_type, request] : requests) {
                    if (req_type == "stat_requests"s) {
                        ReadStatRequests(request, stat_queries);
                    }
                    else if (req_type == "serialization_settings"s) {
                        ReadSerializationSettings(request, path);
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "exception thrown: "s << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unexpected error"s << std::endl;
            }
        }

        ::svg::Color JsonReader::GetColor(const ::json::Node& node) {
            if (node.IsString()) {
                return node.AsString();
            }

            else if (node.IsArray()) {
                const auto& underlayer_color = node.AsArray();
                if (underlayer_color.size() == 3) {
                    return ::svg::Rgb{ static_cast<uint8_t>(underlayer_color[0].AsInt()), static_cast<uint8_t>(underlayer_color[1].AsInt()), static_cast<uint8_t>(underlayer_color[2].AsInt()) };
                }
                else if (underlayer_color.size() == 4) {
                    return ::svg::Rgba{ static_cast<uint8_t>(underlayer_color[0].AsInt()), static_cast<uint8_t>(underlayer_color[1].AsInt()), static_cast<uint8_t>(underlayer_color[2].AsInt()), underlayer_color[3].AsDouble() };
                }
            }

            return { "none" };
        }

        void JsonReader::PrintStatRequests(::renderer::RequestHandler& rh, std::vector<QueryStat>& queries, std::ostream& os) {
            ::json::Array out_array;

            for (const auto& query_out : queries) {
                if (query_out.type == "Bus"s) {
                    PrintBus(rh.GetInfoAboutRoute(query_out.name), query_out.id, out_array);
                }

                if (query_out.type == "Stop"s) {
                    bool contain_stop;
                    set<string_view> buses;
                    std::tie(contain_stop, buses) = rh.GetBusesForStop(query_out.name);
                    PrintStop(contain_stop, buses, query_out.id, out_array);
                }

                if (query_out.type == "Map"s) {
                    PrintMap(rh.RenderMap(), query_out.id, out_array);
                }

                if (query_out.type == "Route"s) {
                    PrintRoute(query_out, out_array, rh);
                }
            }

            json::Print(
                json::Document{
                    json::Builder{}
                    .Value(out_array)
                    .Build()
                },
                os
            );
        }

        void JsonReader::PrintRoute(const QueryStat& query_out, ::json::Array& out_array, ::renderer::RequestHandler& rh) {

            ::transport_router::TransportRouter::RouteInfo route = rh.GetRouteForQuery(query_out);

            if (route.total_time < 0) {
                PrintNotFoundRoute(query_out, out_array);
            }
            else {
                ::json::Array items_array;
                for (const auto& edge_info : route.edges) {

                    if (!edge_info->is_bus_type) {
                        ::json::Node node{
                            json::Builder{}
                                .StartDict()
                                    .Key("stop_name"s).Value(edge_info->stop_name)
                                    .Key("time"s).Value(edge_info->time)
                                    .Key("type"s).Value("Wait"s)
                                .EndDict()
                            .Build()
                        };

                        items_array.emplace_back(move(node));
                    }
                    else if (edge_info->is_bus_type) {
                        ::json::Node node{
                            json::Builder{}
                                .StartDict()
                                    .Key("bus"s).Value(edge_info->bus)
                                    .Key("span_count"s).Value(edge_info->span_count)
                                    .Key("time"s).Value(edge_info->time)
                                    .Key("type"s).Value("Bus"s)
                                .EndDict()
                            .Build()
                        };

                        items_array.emplace_back(move(node));
                    }
                }

                ::json::Node node{
                    json::Builder{}
                        .StartDict()
                            .Key("items"s).Value(items_array)
                            .Key("request_id"s).Value(query_out.id)
                            .Key("total_time"s).Value(route.total_time)
                        .EndDict()
                    .Build()
                };

                out_array.emplace_back(move(node));
            }
        }

        void JsonReader::PrintNotFoundRoute(const QueryStat& query_out, ::json::Array& out_array) {
            ::json::Node node{
                json::Builder{}
                    .StartDict()
                        .Key("error_message"s).Value("not found"s)
                        .Key("request_id"s).Value(query_out.id)
                    .EndDict()
                .Build()
            };
            out_array.emplace_back(move(node));
        }

        void JsonReader::PrintMap(svg::Document doc, int id, ::json::Array& out_array) {
            std::stringstream out;
            doc.Render(out);

            ::json::Node node{
                json::Builder{}
                    .StartDict()
                        .Key("map"s).Value(out.str())
                        .Key("request_id"s).Value(id)
                    .EndDict()
                .Build()
            };

            out_array.emplace_back(move(node));
        }

        void JsonReader::PrintStop(bool contain_stop, std::set<std::string_view>& buses, int id, ::json::Array& out_array) {
            if (!contain_stop) {

                ::json::Node node{
                    json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(id)
                            .Key("error_message"s).Value("not found"s)
                        .EndDict()
                    .Build()
                };

                out_array.emplace_back(move(node));
            }
            else {
                std::vector<std::string> temp{ buses.begin(), buses.end() };

                ::json::Node node{
                    json::Builder{}
                        .StartDict()
                            .Key("buses"s).Value(::json::Array{temp.begin(), temp.end()})
                            .Key("request_id"s).Value(id)
                        .EndDict()
                    .Build()
                };

                out_array.emplace_back(move(node));
            }
        }

        void JsonReader::PrintBus(Bus* bus, int id, ::json::Array& out_array) {
            if (bus->stops_on_route != 0) {
                ::json::Node node{
                    json::Builder{}
                        .StartDict()
                            .Key("curvature"s).Value(bus->curvature)
                            .Key("request_id"s).Value(id)
                            .Key("route_length"s).Value(static_cast<double>(bus->route_length))
                            .Key("stop_count"s).Value(static_cast<int>(bus->stops_on_route))
                            .Key("unique_stop_count"s).Value(static_cast<int>(bus->unique_stops))
                        .EndDict()
                    .Build()
                };

                out_array.emplace_back(move(node));
            }
            else {

                ::json::Node node{
                    json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(id)
                            .Key("error_message"s).Value("not found"s)
                        .EndDict()
                    .Build()
                };

                out_array.emplace_back(move(node));
            }
        }
    }// json_detail
} //namespace directory