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


using Graph = graph::DirectedWeightedGraph<int>;
using Router = graph::Router<int>;

struct EdgeHash{

    size_t operator()(const graph::Edge<int>& edge) const {

        size_t hash1 = std::hash<size_t>{}(edge.from)  ;
        size_t hash2 =  std::hash<size_t>{}(edge.to)  ;
        size_t hash3 = std::hash<int>{}(edge.weight) ;
        

        return hash1 * std::pow(37, 2) + hash2  * std::pow(37, 1) + hash3 * std::pow(37, 3) ;
    }
};


struct RouteWait{
    std::string name;
    float time;
};

struct RouteBus{
    std::string bus;
    int span_count;
    float time;
};


class TransportRouter{
public:

    explicit TransportRouter(){}

    explicit TransportRouter(const TransportCatalogue& catalogue);

    void CreateGraph(const TransportCatalogue& catalogue);

    void InitRouter();

    std::vector<std::variant<RouteBus, RouteWait>> FindRoute(std::string_view stop1, std::string_view stop2) const;

private:

    void AddBus(std::string_view name);

    static float CalculateTime(float velocity, float length);

Graph graph_;

std::shared_ptr<Router> router_;

std::unordered_map<std::string, graph::VertexId> busname_vertexid_;
std::unordered_map<graph::VertexId, std::string> vertexid_busname_;

std::unordered_map<graph::Edge<int>, graph::EdgeId, EdgeHash> edge_index_;
std::unordered_map<graph::EdgeId, graph::Edge<int>> index_edge_;

std::unordered_map<graph::EdgeId, std::string> edgeindex_busname_;

};


}