#include "transport_catalogue.h"
#include <iostream>
#include <exception>

using namespace std;
using namespace ctlg;

void TransportCatalogue::AddBusStop(const BusStop& stop){
    busStop_Database[stop.name] = stop;
    busStop_busRoute_DataBase.insert({busStop_Database[stop.name].name, {}});
}

/// @brief добавление маршрута в базу данных
/// @param route - список остановок
/// @param num  - название маршрута 
/// @param type  - тип маршрута
void TransportCatalogue::AddBusRoute(const std::deque<std::string>& route, string num, BusRoute::Type type){
    BusRoute res;
    res.name = num;
    res.type = type;

    if(busRoute_DataBase.count(num) == 0){
        for(const auto& stop : route){
            res.buses.push_back(CreateBusStop(stop));
        }
        busRoute_DataBase[num] = res;

        for(const auto& stop : busRoute_DataBase[num].buses){
            busStop_busRoute_DataBase[stop->name].insert(busRoute_DataBase[num].name);
        }
    }
}

bool TransportCatalogue::IsBusStop(const std::string &stop) const
{
    return busStop_Database.count(stop) == 1;
}

std::deque<BusStop> TransportCatalogue::GetStops(const string& num) const
{
    if(busRoute_DataBase.count(num) == 0){
        static deque<BusStop> temp;
        return temp;
    }

    std::deque<BusStop> res;
    for(const auto& i : busRoute_DataBase.at(num).buses){
        res.push_back(*i);
    }

    return res;
}


const BusStop* TransportCatalogue::GetStop(const std::string& name) const
{
    return &busStop_Database.at(name);
}

BusRoute TransportCatalogue::GetRoute(string num) const
{

    if(busRoute_DataBase.count(num) == 0){
        return BusRoute();
    }

    return busRoute_DataBase.at(num);
}

size_t TransportCatalogue::GetUniqueStopsForRoute(const string& num) const
{
    // ЕСЛИ NUM ОТСУТСТВУЕТ ?? 

    auto route = busRoute_DataBase.at(num);
    auto type = route.type;

    size_t uniq = 0;

    if(type == BusRoute::Type::CYCLIC){
        unordered_set<string> pasted;
        
        for(auto i : route.buses){
            if(pasted.count(i->name) == 0){
                pasted.insert(i->name);
                uniq++;
            }
        }

        return uniq;
    }
    
    unordered_set<string> pasted;

    for(auto i : route.buses){
        if(pasted.count(i->name) == 0){
            pasted.insert(i->name);
            uniq++;
        }
    }
    return uniq;
    
}

BusRoute::Type  TransportCatalogue::GetRouteType(const string& num) const
{   
    return busRoute_DataBase.at(num).type;
}

std::unordered_set<std::string_view> TransportCatalogue::GetRouteByStop(std::string_view name) const
{
    std::unordered_set<std::string_view> res;
    std::string name_(name);
    if(busStop_Database.count(name_) == 0){
        return res;
    }
    
    std::string name_bus = busStop_Database.at(name_).name;

    for(const auto& route : busStop_busRoute_DataBase.at(name_bus)){
        res.insert(route);
    }

    return res;
}

void TransportCatalogue::SetDistanceBetweenStops(const std::string &stop1, const std::string stop2, int distance)
{
    //cout << "SET " << endl;
    
    CreateBusStop(stop1);
    CreateBusStop(stop2);

    distance_between_stops[{GetStop(stop1), GetStop(stop2)}] = distance;

    //cout << stop1 << " " << stop2 << " " << distance_between_stops[{GetStop(stop1), GetStop(stop2)}]  << endl;
    if(distance_between_stops.count({GetStop(stop2), GetStop(stop1) } )  == 0){
         distance_between_stops[{GetStop(stop2), GetStop(stop1)}] = distance;
    }
}

int TransportCatalogue::GetDistanceBetweenStops(const std::string &stop1, const std::string &stop2) const
{
    if(distance_between_stops.count({GetStop(stop1), GetStop(stop2)}) == 1){
        return distance_between_stops.at({GetStop(stop1), GetStop(stop2)});
    }
    if(distance_between_stops.count({GetStop(stop2), GetStop(stop1)}) == 1){
        return distance_between_stops.at({GetStop(stop2), GetStop(stop1)});
    }

    return 0;
}

BusStop *TransportCatalogue::CreateBusStop(const std::string& name)
{
    if(busStop_Database.count(name) == 0){
        BusStop stop;
        stop.name = name;

        busStop_Database[name] = stop;
        busStop_busRoute_DataBase.insert({busStop_Database[stop.name].name, {}});
    }
    return &busStop_Database.at(name);
}

ctlg::BusStop::BusStop(detail::Coordinates cord, const std::string &name)
{
     coord = cord;
    this->name = std::move(name);
}
