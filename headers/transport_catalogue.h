#pragma once

#include "geo.h"
#include "domain.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace directory {

    class TransportCatalogue {
    public:
        struct Hasher {
            size_t operator()(const std::pair<Stop*, Stop*>& stops) const {
                return (size_t)stops.first + 37 * (size_t)stops.second;
            }
        };

        //добавление маршрута в базу
        void AddRoute(std::string_view bus, std::vector<std::string>& stops, bool is_roundtrip);

        //добавление остановки в базу
        void AddStation(std::string_view stop, ::geo::Coordinates coordinates);
        void AddStationDistance(std::string_view stop, std::unordered_map<std::string, uint64_t, std::hash<std::string_view>>& distance_to_stop);

        //получение информации о маршруте
        //Bus X: R stops on route, U unique stops, L route length
        Bus* GetInfoAboutRoute(std::string_view route);
        std::vector<Bus*> GetBuses();

        //метод для получения списка автобусов по остановке
        std::tuple<bool, std::set<std::string_view>> GetBusesForStop(std::string_view station);

        //задание дистанции между остановками
        void SetDistanceBetweenStops(std::string_view stop_from, std::string_view stop_to, uint64_t distance_to_stop);
        //получение дистанции между остановками
        uint64_t GetDistanceBetweenStops(std::string_view from, std::string_view to);
        
        //Получение списка остановок с координатами для отрисовки
        std::unordered_map<std::string_view, ::geo::Coordinates> GetStopsWithCoordinates();

        std::vector<std::string_view> GetStopsForBus(const std::string_view station) const;
        
        size_t GetCountStops();
        
    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, std::vector<Bus*>> stop_to_bus;
        std::unordered_map<std::string_view, Stop*> stop_to_stop;
        std::unordered_map<std::string_view, std::vector<std::string_view>> bus_to_stops;
        std::unordered_map<std::pair<Stop*, Stop*>, uint64_t, Hasher> distances_;
    };
}