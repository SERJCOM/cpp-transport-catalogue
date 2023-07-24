#include "transport_router.h"
#include <iostream>
#include <cmath>

#include "log_duration.h"

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

    float velocity = catalogue.GetVelocity() / 60.0 ;
    float wait = catalogue.GetWaitTime();

    for(auto name : catalogue.GetRouteNames()){
        
        const BusRoute* route = catalogue.GetRoute(name);

        for(auto it = route->buses.begin(); it != route->buses.end(); it++){
            
            std::string_view name = (*it)->name;

            if(stopname_vertexid_.find(name) == stopname_vertexid_.end()){

                size_t index_f = current_index++;
                size_t index_s = current_index++;
                
                stopname_vertexid_[name] = {index_f, index_s};
                vertexidride_stopname_[index_s] = name;
                vertexidwait_stopname_[index_f] = name;
            }
        }

        
            for(auto it = route->buses.begin(); it != route->buses.end(); it++){
                Edge edge;

                std::string_view name = (*it)->name;

                edge.from = GetStopWait(name);
                edge.to = GetStopRide(name);
                edge.weight = wait;    

                FillGraph(edge);
            }

        if(name == "4"){
            int a = 5;
            a++;
        }


        for(auto it = route->buses.begin(); it != route->buses.end(); it++){
            int span = 0;

            std::string_view it_name = (*it)->name;
            for(auto jt = it + 1; jt != route->buses.end(); jt++){
                std::string_view jt_name = (*jt)->name;

                Edge edge;

                edge.span = span;
                span++;

                    edge.from = GetStopRide(it_name);
                    edge.to = GetStopWait(jt_name);

                    float length = 0;
                    auto it_s = it;
                    for(auto it_e = it + 1; it_e <= jt;  it_e++){
                        length += catalogue.GetDistanceBetweenStops((*(it_s))->name, (*it_e)->name);
                        it_s++;
                    }
                    edge.weight = CalculateTime(velocity, length);
               
                edge.bus = name;

                FillGraph(edge);
            }
        }
        if(route->type == BusRoute::Type::STRAIGHT){
        for(auto it = route->buses.rbegin() ; it != route->buses.rend() ; it++){
            std::string_view it_name = (*it)->name;
            int span = 0; 
                for(auto jt = it + 1; jt != route->buses.rend(); jt++){
                    std::string_view jt_name = (*jt)->name;

                    Edge edge;

                    edge.span = span;
                    span++;

                    edge.from = GetStopRide(it_name);
                    edge.to = GetStopWait(jt_name);

                    float length = 0;
                    auto it_s = it;
                    for(auto it_e = it + 1; it_e <= jt;  it_e++){
                        length += catalogue.GetDistanceBetweenStops((*(it_s))->name, (*it_e)->name);
                        it_s++;
                    }
                    edge.weight = CalculateTime(velocity, length);
                    
                    edge.bus = name;

                    FillGraph(edge);

                }

            }

        }


    }   
}

void ctlg::TransportRouter::InitRouter()
{
    router_ = std::make_shared<Router>(std::ref(graph_));
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
            // Edge<float> edge = index_edge_.at(range);
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



inline float ctlg::TransportRouter::CalculateTime(float velocity, float length)
{
    return (length / 1000) / velocity;   
}
