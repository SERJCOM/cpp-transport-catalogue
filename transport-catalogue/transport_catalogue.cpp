#include "transport_catalogue.h"
#include <iostream>
#include <exception>

using namespace std;
using namespace ctlg;

void TransportCatalogue::AddBusStop(const BusStop& stop){
    if(name_busstop_database.count(stop.name) == 0){
        busstop_database.push_back(stop);
        busstop_busroute_database.insert({busstop_database.back().name, {}});
        name_busstop_database[busstop_database.back().name] = &busstop_database.back();
    }else{
        auto it = std::find_if(busstop_database.begin(), busstop_database.end(), [&](const BusStop& i){
            return (i.name == stop.name);
        });

        it->coord = stop.coord;
    }
        
        
    
}

/// @brief добавление маршрута в базу данных
/// @param route - список остановок
/// @param num  - название маршрута 
/// @param type  - тип маршрута
void TransportCatalogue::AddBusRoute(const std::vector<std::string>& route, string num, BusRoute::Type type){
    BusRoute res;
    res.name = std::move(num);
    res.type = type;



    if(name_busroute_database.count(num) == 0){
        for(const auto& stop : route){
            res.buses.push_back(CreateBusStop(stop));

        }

        busroute_database.push_back(std::move(res));
        name_busroute_database[busroute_database.back().name] = &busroute_database.back(); 


        for(const BusStop* stop : name_busroute_database[busroute_database.back().name]->buses){

            busstop_busroute_database[stop->name].insert(busroute_database.back().name);
        }
        
    }
}

bool TransportCatalogue::BusStopExist(std::string_view stop) const
{
    return name_busstop_database.count(stop) == 1;
}

std::vector<BusStop> TransportCatalogue::GetStops(std::string_view num) const
{
    if(name_busroute_database.count(num) == 0){
        static std::vector<BusStop> temp;
        return temp;
    }

    std::vector<BusStop> res;
    for(const BusStop* i : name_busroute_database.at(num)->buses){
        res.push_back(*i);
    }

    return res;
}


const BusStop* TransportCatalogue::GetStop(std::string_view name) const
{
    if(name_busstop_database.count(name) == 0){
        return nullptr;
    }
    return name_busstop_database.at(name);
}

const BusRoute* TransportCatalogue::GetRoute(std::string_view num) const
{
    if(name_busroute_database.count(num) == 0){
        return nullptr;
    }

    return name_busroute_database.at(num);
}

size_t TransportCatalogue::GetUniqueStopsForRoute(std::string_view num) const
{
    auto route = name_busroute_database.at(num);
    auto type = route->type;

    size_t uniq = 0;

    if(type == BusRoute::Type::CYCLIC){
        unordered_set<string> pasted;
        
        for(auto i : route->buses){
            if(pasted.count(i->name) == 0){
                pasted.insert(i->name);
                uniq++;
            }
        }

        return uniq;
    }
    
    unordered_set<std::string> pasted;

    for(auto i : route->buses){
        if(pasted.count(i->name) == 0){
            pasted.insert(i->name);
            uniq++;
        }
    }
    return uniq;
    
}

BusRoute::Type TransportCatalogue::GetRouteType(std::string_view num) const
{   
    return name_busroute_database.at(num)->type;
}

std::unordered_set<std::string_view>  TransportCatalogue::GetRouteByStop(std::string_view name) const
{
    std::unordered_set<std::string_view> res;
    std::string name_(name);
    if(name_busstop_database.count(name_) == 0){
        return res;
    }
    
    const std::string& name_bus = name_busstop_database.at(name_)->name;

    for(auto route : busstop_busroute_database.at(name_bus)){
        res.insert(name_busroute_database.at(route)->name);
    }

    return res;
}

void TransportCatalogue::SetDistanceBetweenStops(std::string_view stop1, std::string_view stop2, int distance)
{
    auto stop1_ref = CreateBusStop(stop1);
    auto stop2_ref = CreateBusStop(stop2);

    distance_between_stops[{stop1_ref, stop2_ref}] = distance;

    if(distance_between_stops.count({stop2_ref, stop1_ref } )  == 0){
         distance_between_stops[{stop2_ref, stop1_ref}] = distance;
    }
}

int TransportCatalogue::GetDistanceBetweenStops(std::string_view stop1, std::string_view stop2) const
{
    if(distance_between_stops.count({GetStop(stop1), GetStop(stop2)}) == 1){
        return distance_between_stops.at({GetStop(stop1), GetStop(stop2)});
    }
    if(distance_between_stops.count({GetStop(stop2), GetStop(stop1)}) == 1){
        return distance_between_stops.at({GetStop(stop2), GetStop(stop1)});
    }

    return 0;
}

const BusStop *TransportCatalogue::CreateBusStop(std::string_view name)
{
    if(name_busstop_database.count(name) == 0){
        BusStop stop;
        stop.name = std::string(name);

        busstop_database.push_back(std::move(stop));
        name_busstop_database[busstop_database.back().name] = &busstop_database.back();
        busstop_busroute_database.insert({busstop_database.back().name, {}});
    }
    //return name_busstop_database.at(busstop_database.back().name);
    return name_busstop_database.at(name);
}

