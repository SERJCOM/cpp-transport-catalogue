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
        graph_.AddEdge(edge);
    }

    template<typename T>
    void BuildEdge(T begin, T end, std::string_view name, const TransportCatalogue &catalogue){
        for(auto it = begin; it != end; it++){
            int span = 0;
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
                        length += catalogue.GetDistanceBetweenStops((*(it_s))->name, (*it_e)->name);
                        it_s++;
                    }
                    edge.weight = CalculateTime(velocity, length);
               
                edge.bus = name;

                FillGraph(edge);
            }
        }
    }

    Graph graph_;

    std::shared_ptr<Router> router_;

    std::unordered_map<std::string_view, std::pair<VertexOpt, VertexOpt>> stopname_vertexid_;  // wait, ride
    std::unordered_map<graph::VertexId, std::string_view> vertexidwait_stopname_;
    std::unordered_map<graph::VertexId, std::string_view> vertexidride_stopname_;

    int velocity ;
    int wait;


    size_t current_index = 0;

};


}