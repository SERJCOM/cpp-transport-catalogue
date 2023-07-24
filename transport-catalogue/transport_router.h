#pragma once


#include "router.h"
#include <memory>
#include "transport_catalogue.h"
#include <set>
#include <string_view>
#include <unordered_map>
#include "graph.h"
#include <optional>
#include <variant>

namespace ctlg{


using Graph = graph::DirectedWeightedGraph<float>;
using Router = graph::Router<float>;
using VertexOpt = std::optional<graph::VertexId>;

struct EdgeHash{

    size_t operator()(const graph::Edge<float>& edge) const {

        return  std::hash<int>{}(edge.from) * std::pow(37, 2) + std::hash<int>{}(edge.to) ;
    }
};


struct RouteWait{
    std::string_view name;
    float time;
};

struct RouteBus{
    std::string_view name;
    int span_count;
    float time;
};




class TransportRouter{
public:

    using Edge = graph::Edge<float>;

    // explicit TransportRouter(){}

    explicit TransportRouter(const TransportCatalogue& catalogue):graph_(catalogue.GetStopCount() * 2){
        CreateGraph(catalogue);
    };

    void CreateGraph(const TransportCatalogue& catalogue);

    void InitRouter();

    std::optional<std::vector<std::variant<RouteBus, RouteWait>>> FindRoute(std::string_view stop1, std::string_view stop2) const;

private:

    size_t GetStopWait(std::string_view name);
    size_t GetStopRide(std::string_view name);

    static float CalculateTime(float velocity, float length);


    void FillGraph(const Edge& edge){
        // if(edge_index_.find(edge) == edge_index_.end()){
            // edge.index = graph_.GetEdgeCount();
            size_t index = graph_.AddEdge(edge);
            edge_index_[edge] = index;
        // }
    }

Graph graph_;

std::shared_ptr<Router> router_;

std::unordered_map<std::string_view, std::pair<VertexOpt, VertexOpt>> stopname_vertexid_;  // wait, ride
std::unordered_map<graph::VertexId, std::string_view> vertexidwait_stopname_;
std::unordered_map<graph::VertexId, std::string_view> vertexidride_stopname_;

std::unordered_map<graph::Edge<float>, graph::EdgeId, EdgeHash> edge_index_;

std::unordered_map<graph::EdgeId, std::string_view> edgeindex_busname_;

size_t current_index = 0;

};


}