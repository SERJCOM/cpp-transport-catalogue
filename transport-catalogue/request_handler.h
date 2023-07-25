#pragma once

#include "transport_catalogue.h"
#include <memory>
#include <vector>
#include <iostream>
#include "map_renderer.h"
#include <cassert>
#include "transport_router.h"
#include "graph.h"
#include "router.h"

namespace ctlg{



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

    int GetRouteLength(std::string_view name) const ;

    double GetGeoRouteLength(std::string_view name) const ;

    void SetRenderMap(ctlg::MapRenderer* map){
        map_ = map;
    }

    void SetRouter(TransportRouter& router){
        router_ = &router;
    }


    std::optional<std::vector<std::variant<RouteBus, RouteWait>>> GetRoute(std::string_view from, std::string_view to) const ;  

    std::string RenderMap(){
        assert(map_);
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

    void SetVelocity(float velocity){
        assert(catalogue_);

        catalogue_->SetBusVelocity(velocity);
    }

    void SetWaitTime(int time){
        assert(catalogue_);
        catalogue_->SetWaitTime(time);
    }
    
private:

    TransportCatalogue* catalogue_;

    MapRenderer* map_;

    TransportRouter* router_ = nullptr;

};


}