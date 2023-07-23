#pragma once
#include <unordered_map>
#include <string>
#include <unordered_set>
#include "geo.h"
#include <functional>
#include <vector>
#include <string_view>
#include <deque>
#include "domain.h" 

namespace ctlg{


class TransportCatalogue{
public:

    void AddBusStop(const BusStop& stop);

    void AddBusRoute(const std::vector<std::string>& route, std::string num, BusRoute::Type type);

    bool BusStopExist(std::string_view stop) const;

    std::vector<BusStop> GetStops(std::string_view num) const ;

    const BusStop* GetStop(std::string_view name) const ;

    const BusRoute* GetRoute(std::string_view num) const ;

    size_t GetUniqueStopsForRoute(std::string_view num) const ;

    BusRoute::Type GetRouteType(std::string_view num) const ;

    std::unordered_set<std::string_view>  GetRouteByStop(std::string_view name) const ;

    void SetDistanceBetweenStops(std::string_view stop1, std::string_view stop2, int distance);

    int GetDistanceBetweenStops(std::string_view stop1, std::string_view stop2) const ; 


    std::vector<std::string_view> GetRouteNames() const;

    std::vector<const BusRoute*> GetRouteDataBase() const;

    void SetBusVelocity(float velocity){
        bus_velocity_ = velocity;
    }

    float GetVelocity() const {
        return bus_velocity_;
    }

    void SetWaitTime(int time){
        bus_wait_time_ = time;
    }

    int GetWaitTime() const {
        return bus_wait_time_;
    }

    int GetStopCount() const{
        return stop_count_;
    }

private:

    // Нужно для резервирования места для остановки в БД
    const BusStop* CreateBusStop(std::string_view name); 

    
    std::deque<BusStop> busstop_database;
    std::deque<BusRoute> busroute_database;
    std::unordered_map<std::string_view, const BusStop*> name_busstop_database;
    std::unordered_map<std::string_view, const BusRoute*> name_busroute_database;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> busstop_busroute_database; // остановка - ключ, маршрут - значение
    std::unordered_map<std::pair<const BusStop*, const BusStop*>, int, ctlg::Hash::BusA_BusB> distance_between_stops;


    float bus_velocity_;
    int bus_wait_time_;
    int stop_count_ = 0;

};

}