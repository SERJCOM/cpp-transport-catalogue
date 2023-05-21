#pragma once
#include <unordered_map>
#include <string>
#include <unordered_set>
#include "geo.h"
#include <deque>
#include <functional>
#include <vector>
#include <string_view>

namespace ctlg{

struct BusStop{
    BusStop() = default;
    BusStop(detail::Coordinates cord, const std::string& name);

    detail::Coordinates coord;
    std::string name;

    bool exist = false;
};

struct BusRoute{

    enum class Type{
        STRAIGHT,
        CYCLIC
    };

    std::deque<const BusStop*> buses; // это должно быть константным 
    std::string name;
    Type type;
};

namespace Hash{

    struct BusStop{
        size_t operator()(const ctlg::BusStop& stop) const{
            size_t hash = 0;
            hash += std::hash<double>{}(stop.coord.lat) * 37 ^ 3;
            hash += std::hash<double>{}(stop.coord.lng) * 37 ^ 2;
            hash += std::hash<std::string>{}(stop.name) * 37 ;
            return hash;
        }
    };

    struct BusRoute{
        size_t operator()(const ctlg::BusRoute& stop) const{
            return std::hash<std::string>{}(stop.name) * 37 ^ 2;
        }
    };

    //TODO
    // проверить эффективность хеша на имени и попробовать сравнить с хешем на координатах
    struct BusA_BusB{
        size_t operator()(std::pair<const ctlg::BusStop*, const ctlg::BusStop*> pair) const{
            size_t hash = 0;
            hash += std::hash<std::string>{}(pair.first->name) * 37 ^ 2; 
            hash += std::hash<std::string>{}(pair.second->name) * 37;
            return hash;
        }
    };

}


class TransportCatalogue{
public:

    void AddBusStop(const BusStop& stop);

    void AddBusRoute(const std::deque<std::string>& route, std::string num, BusRoute::Type type);

    bool IsBusStop(const std::string& stop) const;

    std::deque<BusStop> GetStops(const std::string& num) const ;

    const BusStop* GetStop(const std::string& name) const ;

    BusRoute GetRoute(std::string num) const ;

    size_t GetUniqueStopsForRoute(const std::string& num) const ;

    BusRoute::Type GetRouteType(const std::string& num) const ;

    std::unordered_set<std::string_view> GetRouteByStop(std::string_view name) const ;

    void SetDistanceBetweenStops(const std::string& stop1, const std::string stop2, int distance);

    int GetDistanceBetweenStops(const std::string& stop1, const std::string& stop2) const ; 

    

private:

    // Нужно для резервирования места для остановки в БД
    BusStop* CreateBusStop(const std::string& name); 

    

    std::unordered_map<std::string, BusStop> busStop_Database;
    std::unordered_map<std::string, BusRoute> busRoute_DataBase;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> busStop_busRoute_DataBase; // остановка - ключ, маршрут - значение
    std::unordered_map<std::pair<const BusStop*, const BusStop*>, int, ctlg::Hash::BusA_BusB> distance_between_stops;
};

}