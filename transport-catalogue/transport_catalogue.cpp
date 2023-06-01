#include "transport_catalogue.h"
#include <iostream>
#include <exception>

using namespace std;
using namespace ctlg;

void TransportCatalogue::AddBusStop(const BusStop& stop){
    busStop_database.push_back(stop);
    name_busStop_database[busStop_database.back().name] = &busStop_database.back();
    busStop_busRoute_dataBase.insert({busStop_database.back().name, {}});
}

/// @brief добавление маршрута в базу данных
/// @param route - список остановок
/// @param num  - название маршрута 
/// @param type  - тип маршрута
void TransportCatalogue::AddBusRoute(const std::vector<std::string>& route, string num, BusRoute::Type type){
    BusRoute res;
    res.name = std::move(num);
    res.type = type;

    if(name_busStop_database.count(num) == 0){
        for(const auto& stop : route){
            res.buses.push_back(CreateBusStop(stop));
        }
        busRoute_database.push_back(res);
        name_busRoute_database[busRoute_database.back().name] = &busRoute_database.back(); 

        for(const BusStop* stop : name_busRoute_database[num]->buses){
            busStop_busRoute_dataBase[stop->name].insert(busRoute_database.back().name);
        }

        // for(const auto stop : name_busRoute_database.at(busRoute_database.back().name).buses){
        //     busStop_busRoute_dataBase[stop->name].insert(busRoute_database.back().name);
        // }
    }
}

bool TransportCatalogue::BusStopExist(std::string_view stop) const
{
    return name_busStop_database.count(stop) == 1;
}

std::vector<BusStop> TransportCatalogue::GetStops(std::string_view num) const
{
    if(name_busRoute_database.count(num) == 0){
        static std::vector<BusStop> temp;
        return temp;
    }

    std::vector<BusStop> res;
    for(const BusStop* i : name_busRoute_database.at(num)->buses){
        res.push_back(*i);
    }

    return res;
}


const BusStop* TransportCatalogue::GetStop(std::string_view name) const
{
    if(name_busStop_database.count(name) == 0){
        return nullptr;
    }
    return name_busStop_database.at(name);
}

BusRoute TransportCatalogue::GetRoute(std::string_view num) const
{
    if(name_busRoute_database.count(num) == 0){
        return BusRoute();
    }

    return *name_busRoute_database.at(num);
}

size_t TransportCatalogue::GetUniqueStopsForRoute(std::string_view num) const
{
    auto route = name_busRoute_database.at(num);
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

BusRoute::Type  TransportCatalogue::GetRouteType(const string& num) const
{   
    return name_busRoute_database.at(num)->type;
}

std::unordered_set<std::string_view> TransportCatalogue::GetRouteByStop(std::string_view name) const
{
    std::unordered_set<std::string_view> res;
    std::string name_(name);
    if(name_busStop_database.count(name_) == 0){
        return res;
    }
    
    const std::string& name_bus = name_busStop_database.at(name_)->name;

    for(const auto& route : busStop_busRoute_dataBase.at(name_bus)){
        res.insert(route);
    }

    return res;
}

void TransportCatalogue::SetDistanceBetweenStops(std::string_view stop1, std::string_view stop2, int distance)
{
    auto stop1_ref = CreateBusStop(stop1);
    auto stop2_ref = CreateBusStop(stop2);

    distance_between_stops[{stop1_ref, stop2_ref}] = distance;

    if(distance_between_stops.count({GetStop(stop2), GetStop(stop1) } )  == 0){
         distance_between_stops[{GetStop(stop2), GetStop(stop1)}] = distance;
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
    if(name_busStop_database.count(std::string(name)) == 0){
        BusStop stop;
        stop.name = name;


        busStop_database.push_back(stop);
        name_busStop_database[busStop_database.back().name] = &busStop_database.back();
        busStop_busRoute_dataBase.insert({busStop_database.back().name, {}});
    }
    return name_busStop_database.at(busStop_database.back().name);
}

ctlg::BusStop::BusStop(Coordinates cord, std::string_view name)
{
    coord = cord;
    this->name = name;
}
