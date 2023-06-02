#pragma once
#include <unordered_map>
#include <string>
#include <unordered_set>
#include "geo.h"
#include <functional>
#include <vector>
#include <string_view>
#include <deque>

namespace ctlg{

struct BusStop{
    BusStop() = default;
    BusStop(Coordinates cord, std::string_view name);

    Coordinates coord;
    std::string name;
};

struct BusRoute{
    enum class Type{
        STRAIGHT,
        CYCLIC
    };

    std::vector<const BusStop*> buses; // это должно быть константным 
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

    void AddBusRoute(const std::vector<std::string>& route, std::string num, BusRoute::Type type);

    bool BusStopExist(std::string_view stop) const;

    std::vector<BusStop> GetStops(std::string_view num) const ;

    const BusStop* GetStop(std::string_view name) const ;

    BusRoute GetRoute(std::string_view num) const ;

    size_t GetUniqueStopsForRoute(std::string_view num) const ;

    BusRoute::Type GetRouteType(std::string_view num) const ;

    std::unordered_set<std::string_view>  GetRouteByStop(std::string_view name) const ;

    void SetDistanceBetweenStops(std::string_view stop1, std::string_view stop2, int distance);

    int GetDistanceBetweenStops(std::string_view stop1, std::string_view stop2) const ; 

    

private:

    // Нужно для резервирования места для остановки в БД
    const BusStop* CreateBusStop(std::string_view name); 

    
    std::deque<BusStop> busStop_database;
    std::deque<BusRoute> busRoute_database;
    std::unordered_map<std::string_view, const BusStop*> name_busStop_database;
    std::unordered_map<std::string_view, const BusRoute*> name_busRoute_database;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> busStop_busRoute_dataBase; // остановка - ключ, маршрут - значение
    std::unordered_map<std::pair<const BusStop*, const BusStop*>, int, ctlg::Hash::BusA_BusB> distance_between_stops;
};

}