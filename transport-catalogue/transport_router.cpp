#include "transport_router.h"
#include <iostream>
#include <cmath>



using namespace ctlg;
using namespace graph;



inline size_t ctlg::TransportRouter::GetStopWait(std::string_view name)
{ 
    return stopname_vertexid_.at(name).first.value();
}

inline size_t ctlg::TransportRouter::GetStopRide(std::string_view name)
{
    return stopname_vertexid_.at(name).second.value();
}

void ctlg::TransportRouter::CreateGraph(const TransportCatalogue &catalogue)
{
    for(auto name : catalogue.GetRouteNames()){
        
        const BusRoute* route = catalogue.GetRoute(name);
        CreateRideWaitStops(*route);
    
        for(auto it = route->stops.begin(); it != route->stops.end(); it++){
            Edge edge;

            std::string_view name = (*it)->name;

            edge.from = GetStopWait(name);
            edge.to = GetStopRide(name);
            edge.weight = GetWaitTime();    

            FillGraph(edge);
        }

        BuildEdge(route->stops.begin(), route->stops.end(), name, catalogue);
        if(route->type == BusRoute::Type::STRAIGHT){
            BuildEdge(route->stops.rbegin(), route->stops.rend(), name, catalogue);
        }
    }   
}

void ctlg::TransportRouter::InitRouter()
{
    if(!router_)
        CreateRouter();
    router_->InitialRouter();
}


std::optional<std::vector<std::variant<RouteBus, RouteWait>>> ctlg::TransportRouter::FindRoute(std::string_view stop1, std::string_view stop2) const
{
    if(stopname_vertexid_.find(stop1) == stopname_vertexid_.end() || stopname_vertexid_.find(stop2) == stopname_vertexid_.end()){
        return std::nullopt;
    }   

    std::optional<graph::Router<float>::RouteInfo> route = router_->BuildRoute(stopname_vertexid_.at(stop1).first.value(), stopname_vertexid_.at(stop2).first.value());    
    
    if(route.has_value()){

        std::vector<std::variant<RouteBus, RouteWait>> res;

        res.reserve(route.value().edges.size());

        for(auto range : route->edges){
            Edge edge = graph_.GetEdge(range);

            VertexId from = edge.from;
            VertexId to = edge.to;

            auto find_name = [&](VertexId id){
                if(vertexidwait_stopname_.find(id) != vertexidwait_stopname_.end()){
                    return vertexidwait_stopname_.at(id);
                }
                return vertexidride_stopname_.at(id);
            };

            std::string_view from_name = find_name(from);
            std::string_view to_name = find_name(to);

            if(from_name == to_name){
                RouteWait wait;
                wait.name = from_name;
                wait.time = edge.weight;
                res.push_back(wait);
            } else{
                RouteBus bus;

                bus.name = edge.bus;
                bus.span_count = edge.span;
                bus.time = edge.weight;
                res.push_back(bus);
            }
        }

    return res;

    } else{
        return std::nullopt;
    }
}

void ctlg::TransportRouter::SetVelocity(float velocity)
{
    settings_.velocity= velocity;
}

float ctlg::TransportRouter::GetVelocity() const
{
    return settings_.velocity;
}

void ctlg::TransportRouter::SetWaitTime(int wait)
{
    settings_.wait = wait;
}

const Graph &ctlg::TransportRouter::GetGraph() const
{
    return graph_;
}

Graph &ctlg::TransportRouter::GetGraph()
{
    return graph_;
}

const ctlg::Router &ctlg::TransportRouter::GetRouter() const
{
    return *router_;
}

ctlg::Router &ctlg::TransportRouter::GetRouter()
{
    return *router_;
}

inline float ctlg::TransportRouter::CalculateTime(float velocity, float length)
{
    return (length / 1000) / velocity * 60;   
}

void ctlg::TransportRouter::CreateRideWaitStops(const BusRoute &route)
{
    for(auto it = route.stops.begin(); it != route.stops.end(); it++){
            
        std::string_view name = (*it)->name;

        if(stopname_vertexid_.find(name) == stopname_vertexid_.end()){

            size_t index_f = current_index++;
            size_t index_s = current_index++;
            
            stopname_vertexid_[name] = {index_f, index_s};
            vertexidride_stopname_[index_s] = name;
            vertexidwait_stopname_[index_f] = name;
        }
    }
}
