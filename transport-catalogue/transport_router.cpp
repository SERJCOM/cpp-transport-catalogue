#include "transport_router.h"
#include <iostream>
#include <cmath>

#include "log_duration.h"

using namespace ctlg;
using namespace graph;



size_t ctlg::TransportRouter::AddBusWait(std::string_view name)
{
   
    if(stopname_vertexid_.find(name) == stopname_vertexid_.end() || !stopname_vertexid_.at(name).first.has_value()){
        stopname_vertexid_[name].first = current_index;
        vertexidwait_stopname_[current_index] = name;
        current_index++;
    } 


    return stopname_vertexid_.at(name).first.value();

}

size_t ctlg::TransportRouter::AddBusRide(std::string_view name)
{
    if(stopname_vertexid_.find(name) == stopname_vertexid_.end() || !stopname_vertexid_.at(name).second.has_value()){
        stopname_vertexid_[name].second = current_index;
        vertexidride_stopname_[current_index] = name;
        current_index++;
    } 

    return stopname_vertexid_.at(name).second.value();

}

void ctlg::TransportRouter::CreateGraph(const TransportCatalogue &catalogue)
{
    auto names = catalogue.GetRouteNames();


    for(auto name : names){
        
        const BusRoute* route = catalogue.GetRoute(name);
        

        for(auto it = route->buses.begin(); it != route->buses.end(); it++){
            for(auto jt = it; jt != route->buses.end(); jt++){

                Edge<float> edge;

                if(it == jt){
                    edge.from = AddBusWait((*it)->name);
                    edge.to = AddBusRide((*jt)->name);
                }
                else{
                    edge.from = AddBusRide((*it)->name);
                    edge.to = AddBusWait((*jt)->name);
                }

                edge.span = std::distance(it, jt);

                if(it != jt){
                    float length = 0;
                        auto it_s = it;
                        for(auto it_e = it + 1; it_e <= jt;  it_e++){

                            length += catalogue.GetDistanceBetweenStops((*(it_s))->name, (*it_e)->name);
                            it_s++;
                        }
                    

                    float velocity = catalogue.GetVelocity() / 60.0 ;
                    edge.weight = CalculateTime(velocity, length);
                
                }   else{
                    edge.weight = catalogue.GetWaitTime();
                }

                    if(edge_index_.count(edge) == 0){
                    size_t index = graph_.AddEdge(edge);
                    edge_index_[edge] = index;
                    // index_edge_[index] = edge;

                    edgeindex_busname_[index] = route->name;
                }
            }
        }

    }   
}

void ctlg::TransportRouter::InitRouter()
{
    router_ = std::make_shared<Router>(graph_);
}



std::optional<std::vector<std::variant<RouteBus, RouteWait>>> ctlg::TransportRouter::FindRoute(std::string_view stop1, std::string_view stop2) const
{

    if(stopname_vertexid_.count(stop1) == 0 || stopname_vertexid_.count(stop2) == 0){
        return std::nullopt;
    }
    

    std::optional<graph::Router<float>::RouteInfo> route = router_->BuildRoute(stopname_vertexid_.at(stop1).first.value(), stopname_vertexid_.at(stop2).first.value());    
    
    if(route.has_value()){

        std::vector<std::variant<RouteBus, RouteWait>> res;

        res.reserve(route.value().edges.size());

        for(auto range : route->edges){
            // Edge<float> edge = index_edge_.at(range);
            Edge<float> edge = graph_.GetEdge(range);

            VertexId from = edge.from;
            VertexId to = edge.to;


            auto find_name = [&](VertexId id){
                if(vertexidwait_stopname_.count(id) != 0){
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

                bus.name = edgeindex_busname_.at(edge_index_.at(edge));
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
