#pragma once

#include<string_view>
#include "geo.h"
#include <string>
#include <vector>

namespace ctlg{

struct BusStop{
    BusStop() = default;
    BusStop(geo::Coordinates cord, std::string_view name);
    BusStop(geo::Coordinates cord, std::string name);

    geo::Coordinates coord;
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

}