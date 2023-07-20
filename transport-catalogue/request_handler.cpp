#include <utility>
#include <iostream>
#include "request_handler.h"
#include <iomanip>
#include <set>
#include <algorithm>


using namespace ctlg;


int ctlg::RequestHandler::GetRouteLength(std::string_view name) const
{
    int length = 0;

    auto stops = catalogue_->GetStops(name);

    for(auto it = stops.begin(); it != stops.end() - 1; it++){
        length += catalogue_->GetDistanceBetweenStops(it->name, (it + 1)->name);
    }

    if(catalogue_->GetRoute(name)->type == BusRoute::Type::STRAIGHT){
        for(auto it = stops.end() - 1; it != stops.begin(); it--){
            length += catalogue_->GetDistanceBetweenStops(it->name, (it - 1)->name);
        }
    }

    return length;
}

double ctlg::RequestHandler::GetGeoRouteLength(std::string_view name) const
{
    double length = 0;

    auto stops = catalogue_->GetStops(name);

    for(auto it = stops.begin(); it != stops.end() - 1; it++){
        length += geo::ComputeDistance(it->coord, (it + 1)->coord);
    }

    if(catalogue_->GetRoute(name)->type == BusRoute::Type::STRAIGHT){
        length *= 2;
    }


    return length;
    
}

std::vector<std::variant<RouteBus, RouteWait>> ctlg::RequestHandler::GetRoute(std::string_view from, std::string_view to) const
{
    std::vector<std::variant<RouteBus, RouteWait>> temp = router_->FindRoute(from, to);

    std::vector<std::variant<RouteBus, RouteWait>> res;

    auto it = temp.begin(); 

    while(it != temp.end()){

        if(std::holds_alternative<RouteWait>(*it)){
            res.push_back(std::get<RouteWait>(*it));
            it++;
        } 

        else{
            auto temp_it = std::adjacent_find(it, temp.end(), [&](const auto& elem1, const auto& elem2){
                return std::holds_alternative<RouteBus>(elem1) && std::holds_alternative<RouteBus>(elem2);
            });

            RouteBus bus = std::get<RouteBus>(*it);
            bus.span_count = std::distance(temp_it, it) - 1;
            res.push_back(bus);
            it = temp_it;
        }

        

    }


    return res;
}

TransportCatalogue *ctlg::RequestHandler::GetCatalogue() const
{
    return catalogue_;
}



void ctlg::RequestHandler::SetStop(const BusStop &stop)
{
    catalogue_->AddBusStop(stop);
}

void ctlg::RequestHandler::SetBus(const BusRoute &route, const std::vector<std::string> &stops)
{    
    catalogue_->AddBusRoute(stops, route.name, route.type);
}

void ctlg::RequestHandler::SetDistance(std::string_view stop1, std::string_view stop2, double distance){
    catalogue_->SetDistanceBetweenStops(stop1, stop2, distance);
}




