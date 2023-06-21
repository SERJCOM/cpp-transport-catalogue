#include <utility>
#include <iostream>
#include "request_handler.h"
#include <iomanip>
#include <set>


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




