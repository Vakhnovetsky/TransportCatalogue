#include "transport_catalogue.h"

namespace directory {

    using namespace std;

    //добавление маршрута в базу
    void TransportCatalogue::AddRoute(std::string_view bus, std::vector<std::string>& stops, bool is_roundtrip) {
        std::string name = { bus.begin(), bus.end() };
        size_t stops_on_route = 0;
        size_t coefficient = 0;

        if (is_roundtrip) {
            //круговой
            stops_on_route = stops.size();
            coefficient = 1;
        }
        else {
            //туда-обратно
            stops_on_route = stops.size() * 2 - 1;
            coefficient = 2;
        }

        set temp_stops(stops.begin(), stops.end());
        buses_.emplace_back(name, stops_on_route, temp_stops.size(), is_roundtrip);
        const string_view bus_name = buses_.back().bus_name;

        std::vector<std::string_view> vector_stops;
        for (const auto& s : stops) {
            vector_stops.push_back(stop_to_stop.at(s)->station_name);
        }

        bus_to_stops[bus_name] = vector_stops;

        double length = 0;
        uint64_t route_length = 0;
        ::geo::Coordinates from;
        ::geo::Coordinates to;
        bool flag = false;

        for (auto iter = bus_to_stops.at(bus_name).begin(); iter != bus_to_stops.at(bus_name).end(); ++iter) {
            auto iter_next = std::next(iter);
            if (iter_next != bus_to_stops.at(bus_name).end()) {
                route_length += GetDistanceBetweenStops(*iter, *iter_next);
            }

            for (const Stop& s : stops_) {
                if (s.station_name == *iter) {
                    if (flag) {
                        from = to;
                        to = s.coordinates;
                        length += std::abs(::geo::ComputeDistance(from, to));
                    }
                    else {
                        flag = true;
                        to = s.coordinates;
                    }
                    break;
                }
            }
            stop_to_bus[*iter].push_back(&buses_.back());
        }

        if (coefficient == 2) {
            //The bus needs to turn around and go back.
            route_length += GetDistanceBetweenStops(*(bus_to_stops.at(bus_name).end() - 1), *(bus_to_stops.at(bus_name).end() - 1));
            flag = false;

            for (auto iter = bus_to_stops.at(bus_name).rbegin(); iter != bus_to_stops.at(bus_name).rend(); ++iter) {
                auto iter_next = std::next(iter);

                if (iter_next != bus_to_stops.at(bus_name).rend()) {
                    route_length += GetDistanceBetweenStops(*iter, *iter_next);
                }
            }
        }

        buses_.back().route_length = route_length;
        buses_.back().curvature = route_length / (length * coefficient);
    }

    //добавление остановки в базу
    void TransportCatalogue::AddStation(string_view stop, ::geo::Coordinates coordinates) {
        stops_.emplace_back(stop, coordinates);

        string_view curr_stop = stops_.back().station_name;
        stop_to_stop[curr_stop] = &stops_.back();
        stop_to_bus[curr_stop] = {};
    }

    void TransportCatalogue::AddStationDistance(string_view stop, unordered_map<string, uint64_t, std::hash<std::string_view>>& distance_to_stop) {
        for (const auto& [destination_stop, distance] : distance_to_stop) {
            SetDistanceBetweenStops(stop, destination_stop, distance);
        }
    }

    //получение информации о маршруте
    //Bus X: R stops on route, U unique stops, L route length
    Bus* TransportCatalogue::GetInfoAboutRoute(string_view route) {
        for (Bus& bus : buses_) {
            if (bus.bus_name == route) {
                return &bus;
            }
        }
        static Bus empty_bus("", 0, 0, false);
        return &empty_bus;
    }
    
    vector<Bus*> TransportCatalogue::GetBuses() {
        vector<Bus*> result;
        for (Bus& bus : buses_) {
            result.push_back(&bus);
        }

        std::sort(result.begin(), result.end(), [](const Bus* lhs, const Bus* rhs) { 
            return lhs->bus_name < rhs->bus_name; 
        });

        return result;
    }

    vector<string_view> TransportCatalogue::GetStopsForBus(const std::string_view station) const {
        return bus_to_stops.at(station);
    }
    
    //метод для получения списка автобусов по остановке
    tuple<bool, set<string_view> > TransportCatalogue::GetBusesForStop(string_view station) {
        if (stop_to_bus.count(station) > 0) {
            set<string_view> retult;
            for (auto& bus : stop_to_bus.at(station)) {
                retult.insert(bus->bus_name);
            }
            return { true , retult };
        }
        else {
            return { false , {} };
        }
    }

    uint64_t TransportCatalogue::GetDistanceBetweenStops(std::string_view from, std::string_view to) {
        if (distances_.count({ stop_to_stop.at(from), stop_to_stop.at(to) })>0) {
            return distances_.at({ stop_to_stop.at(from), stop_to_stop.at(to) });
        }
        
        if (distances_.count({ stop_to_stop.at(to), stop_to_stop.at(from) }) > 0) {
            return distances_.at({ stop_to_stop.at(to), stop_to_stop.at(from) });
        }

        return 0;
    }
    
    void TransportCatalogue::SetDistanceBetweenStops(std::string_view stop_from, std::string_view stop_to, uint64_t distance_to_stop) {
        distances_[pair(stop_to_stop[stop_from], stop_to_stop[stop_to])] = distance_to_stop;
    }

    unordered_map<string_view, ::geo::Coordinates> TransportCatalogue::GetStopsWithCoordinates() {
        unordered_map<string_view, ::geo::Coordinates> result;
        for (const auto& [bus, stops] : bus_to_stops) {
            for (const auto& stop : stops) {
                result[stop] = stop_to_stop.at(stop)->coordinates;
            }
        }
        return result;
    }
    
    size_t TransportCatalogue::GetCountStops() {
        return stops_.size();
    }
}