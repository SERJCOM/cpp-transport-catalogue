#include "transport_router.h"
#include <iostream>

using namespace ctlg;
using namespace graph;
using namespace std;

ctlg::TransportRouter::TransportRouter(const TransportCatalogue &catalogue)
{
    CreateGraph(catalogue);
}

//TODO//
/*  

    СЧИТАТЬ ВРЕМЯ ДВИЖЕНИЯ В МИНУТАХ, ТО ЕСТЬ ПЕРЕВЕСТИ СКОРОСТЬ В КМ В МИН,

*/

void ctlg::TransportRouter::CreateGraph(const TransportCatalogue &catalogue)
{
    auto names = catalogue.GetRouteNames();


    for(auto name : names){

        // std::cout << name << std::endl;
        
        const BusRoute* route = catalogue.GetRoute(name);

        // int i = 0;

        for(auto it = route->buses.begin() + 1; it != route->buses.end(); it++){
            graph::Edge<int> edge;


            AddBus((*(it - 1))->name);
            edge.from = busname_vertexid_[(*(it - 1))->name];


            Edge<int> wait;
            wait.from = edge.from;
            wait.to = edge.from;
            wait.weight = catalogue.GetWaitTime();

            size_t wait_index = graph_.AddEdge(wait);

            edge_index_[wait] = wait_index;
            index_edge_[wait_index] = wait;

            
            AddBus((*it)->name); 
            edge.to = busname_vertexid_[(*it)->name];


            float length = catalogue.GetDistanceBetweenStops((*(it - 1))->name, (*it)->name);
            float velocity = catalogue.GetVelocity();


            edge.weight = CalculateTime(velocity, length);
            

            size_t index = graph_.AddEdge(edge);
            edge_index_[edge] = index;
            index_edge_[index] = edge;

            // std::cout << "5" << std::endl;

            edgeindex_busname_[index] = route->name;

            // std::cout << "6" << std::endl;


            // i++;
        }

    }   
}

void ctlg::TransportRouter::InitRouter()
{
    router_ = std::make_shared<Router>(graph_);
}

std::vector<std::variant<RouteBus, RouteWait>> ctlg::TransportRouter::FindRoute(std::string_view stop1, std::string_view stop2) const
{
    std::optional<graph::Router<int>::RouteInfo> route = router_->BuildRoute(busname_vertexid_.at(string(stop1)), busname_vertexid_.at(string(stop2)));
    if(route.has_value()){

        std::vector<std::variant<RouteBus, RouteWait>> res;

        for(auto range : route->edges){
            Edge<int> edge = index_edge_.at(range);

            VertexId from = edge.from;
            VertexId to = edge.to;

            if(from == to){
                RouteWait wait;
                wait.name = vertexid_busname_.at(from);
                wait.time = edge.weight;

                res.push_back(wait);
            } else{
                RouteBus bus;

                bus.bus = edgeindex_busname_.at(edge_index_.at(edge));
                bus.span_count = 1;
                bus.time = edge.weight;

                res.push_back(bus);
            }

        }

    return res;


    } else{
        return std::vector<std::variant<RouteBus, RouteWait>>{};
    }
}


void ctlg::TransportRouter::AddBus(std::string_view name)
{
    if(busname_vertexid_.count(name) == 0){
        size_t index = busname_vertexid_.size();
        busname_vertexid_[name] = index;
        vertexid_busname_[index] = name;
    }
}

float ctlg::TransportRouter::CalculateTime(float velocity, float length)
{
    return length / velocity;   
}
