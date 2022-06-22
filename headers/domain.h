#pragma once

#include "geo.h"

#include <string_view>
#include <string>
#include <unordered_map>
#include <vector>

namespace directory {

    struct Stop {
        std::string station_name;
        ::geo::Coordinates coordinates;

        Stop(std::string_view p_station_name, ::geo::Coordinates p_coordinates)
            : station_name({ p_station_name.begin(), p_station_name.end() }),
            coordinates(p_coordinates) {
        }

        Stop(Stop&& other) noexcept
            : station_name(other.station_name),
            coordinates(other.coordinates) {
        }
        Stop& operator=(const Stop& other) = default;
    };

    struct Bus {
        std::string bus_name;
        size_t stops_on_route;
        size_t unique_stops;
        uint64_t route_length = 0;
        double curvature = 0;
        bool is_roundtrip = false;

        Bus(std::string p_bus_name, size_t p_stops_on_route, size_t p_unique_stops, bool p_is_roundtrip)
            : bus_name(std::move(p_bus_name)),
            stops_on_route(p_stops_on_route),
            unique_stops(p_unique_stops),
            is_roundtrip(p_is_roundtrip) {
        }

        Bus(Bus&& other) noexcept
            : bus_name(other.bus_name),
            stops_on_route(other.stops_on_route),
            unique_stops(other.unique_stops),
            route_length(other.route_length),
            curvature(other.curvature),
            is_roundtrip(other.is_roundtrip) {
        }
        Bus& operator=(const Bus& other) = default;
    };

    namespace json_detail {

        struct QueryBus {
            std::string bus;
            std::vector<std::string> stops;
            bool is_roundtrip = false;
        };

        struct QueryStop {
            std::string stop;
            ::geo::Coordinates coordinates;
            std::unordered_map<std::string, uint64_t, std::hash<std::string_view>> distance_to_stop;
        };

        struct QueryStat {
            int id = 0;
            std::string type; //Route or Stop or Bus
            std::string name; //for Stop and Bus
            std::string from; //for Route
            std::string to; //for Route
        };
    }
}