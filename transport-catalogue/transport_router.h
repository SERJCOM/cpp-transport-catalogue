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


namespace serialize{
class Deserialization;
class Serialization;
}

namespace ctlg{


using Graph = graph::DirectedWeightedGraph<float>;
using Router = graph::Router<float>;
using VertexOpt = std::optional<graph::VertexId>;



struct RouteWait{
    std::string_view name;
    float time;
};

struct RouteBus{
    std::string_view name;
    int span_count;
    float time;
};

struct RouteSettings{
    float velocity ;
    int wait;

    std::unordered_map<std::string_view, std::pair<graph::VertexId, graph::VertexId>> stopname_vertexid;  // wait, ride
    std::unordered_map<graph::VertexId, std::string_view> vertexidwait_stopname;
    std::unordered_map<graph::VertexId, std::string_view> vertexidride_stopname;
};



class TransportRouter{

    public:

    using Edge = graph::Edge<float>;

    explicit TransportRouter(const TransportCatalogue& catalogue):graph_(catalogue.GetStopCount() * 2) {
        CreateGraph(catalogue);
    };

    explicit TransportRouter() = default;

    void InitGraph(const TransportCatalogue& catalogue){
        graph_.SetVertexCount(catalogue.GetStopCount() * 2);
        CreateGraph(catalogue);
    }

    void CreateRouter(){
        router_ = std::make_shared<Router>(std::ref(graph_));
    }

    void InitRouter();


    std::optional<std::vector<std::variant<RouteBus, RouteWait>>> FindRoute(std::string_view stop1, std::string_view stop2) const;

    void SetVelocity(float velocity);

    float GetVelocity() const;

    void SetWaitTime(int wait);

    const Graph& GetGraph() const;

    Graph& GetGraph();    

    const Router& GetRouter() const;

    Router& GetRouter();

    int GetWaitTime() const{
        return settings_.wait;
    }

    const std::unordered_map<std::string_view, std::pair<VertexOpt, VertexOpt>>& GetStopsVertex() const {
        return stopname_vertexid_;
    }

    const std::unordered_map<graph::VertexId, std::string_view>& GetVertexWaitStopsName() const {
        return vertexidwait_stopname_;
    }

    const std::unordered_map<graph::VertexId, std::string_view>& GetVertexRideStopsName() const {
        return vertexidride_stopname_;
    }


    private:

    friend class serialize::Deserialization;
    friend class serialize::Serialization;

    void CreateGraph(const TransportCatalogue& catalogue);

    size_t GetStopWait(std::string_view name);
    size_t GetStopRide(std::string_view name);

    static float CalculateTime(float velocity, float length);

    void FillGraph(const Edge& edge){
        graph_.AddEdge(edge);
    }

    void CreateRideWaitStops(const BusRoute& route);

    template<typename T>
    void BuildEdge(T begin, T end, std::string_view name, const TransportCatalogue &catalogue);

    Graph graph_;

    std::shared_ptr<Router> router_;

    std::unordered_map<std::string_view, std::pair<VertexOpt, VertexOpt>> stopname_vertexid_;  // wait, ride
    std::unordered_map<graph::VertexId, std::string_view> vertexidwait_stopname_;
    std::unordered_map<graph::VertexId, std::string_view> vertexidride_stopname_;

    RouteSettings settings_;

    size_t current_index = 0;
};

template <typename T>
void TransportRouter::BuildEdge(T begin, T end, std::string_view name, const TransportCatalogue &catalogue)
{
    for(auto it = begin; it != end; it++){
        int span = 1;

        std::string_view it_name = (*it)->name;
        for(auto jt = it + 1; jt != end; jt++){
            std::string_view jt_name = (*jt)->name;

            Edge edge;

            edge.span = span;
            span++;

            edge.from = GetStopRide(it_name);
            edge.to = GetStopWait(jt_name);

            float length = 0;
            auto it_s = it;
            for(auto it_e = it + 1; it_e <= jt;  it_e++){
                length += catalogue.GetOneWayDistance((*(it_s))->name, (*it_e)->name);
                it_s++;
            }
            edge.weight = CalculateTime(settings_.velocity, length);
        
            edge.bus = name;

            FillGraph(edge);
        }
    }
}

}