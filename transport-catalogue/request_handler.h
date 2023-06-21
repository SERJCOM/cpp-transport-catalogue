#pragma once

#include "transport_catalogue.h"
#include <memory>
#include <vector>
#include <iostream>
#include "map_renderer.h"
#include <cassert>

namespace ctlg{


void GetRouteInfo(std::ostream& out, std::istream& in, const ctlg::TransportCatalogue& ctl);

class RequestHandler{
public:
    RequestHandler() = default;

    RequestHandler(TransportCatalogue* catalogue):catalogue_(catalogue){}

    void SetCatalogue(TransportCatalogue* catalogue){
        catalogue_ = catalogue;
    }

    void SetStop(const BusStop& stop);

    void SetBus(const BusRoute& route, const std::vector<std::string>& stops);

    void SetDistance(std::string_view stop1, std::string_view stop2, double distance);

    void AddNewNode(const std::string& str);

    int GetRouteLength(std::string_view name) const ;

    double GetGeoRouteLength(std::string_view name) const ;

    void SetRenderMap(ctlg::MapRenderer* map){
        map_ = map;
    }

    std::string RenderMap(){
        assert(map_ != nullptr);
        std::ostringstream str;
       
        auto routes = catalogue_->GetRouteDataBase();

        std::sort(routes.begin(), routes.end(), [&](auto route1, auto route2){
            return route1->name < route2->name;
        });

        map_->DrawMap(str, routes);

        return str.str();
    }

    
    ctlg::MapRenderer* GetRenderMap(){
        return map_;
    }

    TransportCatalogue* GetCatalogue() const ;
    
private:

TransportCatalogue* catalogue_;

MapRenderer* map_;

};


}
