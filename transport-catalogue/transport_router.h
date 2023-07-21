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

struct EdgeHash{

    size_t operator()(const graph::Edge<float>& edge) const {

        size_t hash1 = std::hash<size_t>{}(edge.from)  ;
        size_t hash2 =  std::hash<size_t>{}(edge.to)  ;
        size_t hash3 = std::hash<float>{}(edge.weight) ;
        

        return hash1 * std::pow(37, 2) + hash2  * std::pow(37, 1) + hash3 * std::pow(37, 3) ;
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


struct Wait{
    std::string_view name;


    bool operator==(const Wait& wait) const{
        return name == wait.name;
    }
};

struct Ride{
    std::string_view name;


    bool operator==(const Ride& wait) const{
        return name == wait.name;
    }
};

struct WaitRideHash{

    size_t operator()(const std::variant<Wait, Ride>& var) const{
        std::string name;

        std::visit([&name](auto value){
            name = std::string(value.name);
        }, var);

        size_t salt = 37;

        if(std::holds_alternative<Wait>(var)){
            salt = 37 * 37;
        }


        return std::hash<std::string>{}(name) * salt;
    }

};


class TransportRouter{
public:

    // explicit TransportRouter(){}

    explicit TransportRouter(const TransportCatalogue& catalogue):graph_(catalogue.GetStopCount() * 2){
        CreateGraph(catalogue);
    };

    void CreateGraph(const TransportCatalogue& catalogue);

    void InitRouter();

    std::optional<std::vector<std::variant<RouteBus, RouteWait>>> FindRoute(std::string_view stop1, std::string_view stop2) const;

private:

    size_t AddBus(std::variant<Wait, Ride> name);

    static float CalculateTime(float velocity, float length);

Graph graph_;

std::shared_ptr<Router> router_;

std::unordered_map<std::variant<Wait, Ride>, graph::VertexId, WaitRideHash> busname_vertexid_;
std::unordered_map<graph::VertexId, std::variant<Wait, Ride>> vertexid_busname_;

std::unordered_map<graph::Edge<float>, graph::EdgeId, EdgeHash> edge_index_;
std::unordered_map<graph::EdgeId, graph::Edge<float>> index_edge_;

std::unordered_map<graph::EdgeId, std::string_view> edgeindex_busname_;

};


}