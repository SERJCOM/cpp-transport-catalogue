#include "transport_router.h"
#include <iostream>

using namespace ctlg;
using namespace graph;




void ctlg::TransportRouter::CreateGraph(const TransportCatalogue &catalogue)
{
    auto names = catalogue.GetRouteNames();


    for(auto name : names){
        
        const BusRoute* route = catalogue.GetRoute(name);


        auto it_final =  route->buses.end();


        for(auto it = route->buses.begin(); it != it_final; it++){
            for(auto jt = it; jt != it_final; jt++){

                Edge<float> edge;

                if(*it == *jt){
                    edge.from = AddBus(Wait{(*it)->name});
                    edge.to = AddBus(Ride{(*jt)->name});
                }
                else{
                    edge.from = AddBus(Ride{(*it)->name});
                    edge.to = AddBus(Wait{(*jt)->name});
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
                    index_edge_[index] = edge;

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
    std::optional<graph::Router<float>::RouteInfo> route = router_->BuildRoute(busname_vertexid_.at(Wait{stop1}), busname_vertexid_.at(Wait{stop2}));
    
    
    if(route.has_value()){

        std::vector<std::variant<RouteBus, RouteWait>> res;

        for(auto range : route->edges){
            Edge<float> edge = index_edge_.at(range);

            VertexId from = edge.from;
            VertexId to = edge.to;

            std::string_view from_name;
            std::string_view to_name;

            std::visit([&from_name](auto value){
                from_name = value.name;
            }, vertexid_busname_.at(from));

            std::visit([&to_name](auto value){
                to_name = value.name;
            }, vertexid_busname_.at(to));

            if(from_name == to_name){
                RouteWait wait;

                std::visit( [&wait](auto value) { wait.name = value.name; }, vertexid_busname_.at(from));

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


size_t ctlg::TransportRouter::AddBus(std::variant<Wait, Ride> name)
{
    if(busname_vertexid_.count(name) == 0){
        size_t index = busname_vertexid_.size();
        busname_vertexid_[name] = index;
        vertexid_busname_[index] = name;

        return index;
    }

    return busname_vertexid_.at(name);
}

float ctlg::TransportRouter::CalculateTime(float velocity, float length)
{
    return (length / 1000) / velocity;   
}
