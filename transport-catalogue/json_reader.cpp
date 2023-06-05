#include "json_reader.h"
#include <string>
#include <set>

using namespace ctlg;

void ctlg::JsonReader::LoadDocument(std::istream &input)
{
    doc_ = json::Load(input);
}

void ctlg::JsonReader::SetRequest(Request &request)
{
    auto base_requests = doc_.GetRoot().AsDict().at("base_requests").AsArray();
    for(const auto& node : base_requests){
        const auto nodedict = node.AsDict();
        if(nodedict.at("type").AsString() == "Bus"){
            BusRoute bus;

           std::string name = nodedict.at("name").AsString();

           std::vector<std::string> stops;
           if(nodedict.at("is_roundtrip").AsBool()){
            bus.type = ctlg::BusRoute::Type::CYCLIC;
           }
           else{
            bus.type = ctlg::BusRoute::Type::STRAIGHT;
           }
           
           for(auto i : nodedict.at("stops").AsArray()){
                stops.push_back(i.AsString());
           }

           request.GetCatalogue()->AddBusRoute(stops, std::move(name), bus.type);
        }
        else if(nodedict.at("type").AsString() == "Stop"){
            std::string name = nodedict.at("name").AsString();
            double latitude = nodedict.at("latitude").AsDouble();
            double longitude = nodedict.at("longitude").AsDouble();
            json::Dict distance = nodedict.at("road_distances").AsDict();
            BusStop stop({latitude, longitude}, name);

            request.GetCatalogue()->AddBusStop(stop);
            for(auto i : distance){
                request.GetCatalogue()->SetDistanceBetweenStops(name, i.first, i.second.AsDouble()) ;
            }
        }
    }
    

}

void ctlg::JsonReader::GetInformation(std::ostream &output, Request &request){

    auto stat_requests = doc_.GetRoot().AsDict().at("stat_requests").AsArray();

    json::Array result;

    for(auto i : stat_requests){
        if(i.AsDict().at("type").AsString() == "Stop"){
            auto node = i.AsDict();

            int id = node.at("id").AsInt();
            std::string name = node.at("name").AsString();
            json::Dict answer;
            if(request.GetCatalogue()->BusStopExist(name)){
               json::Array buses ;
               const auto& set = request.GetCatalogue()->GetRouteByStop(name);
                for(const auto& str : set){
                    buses.push_back(std::string(str));
                }
                std::sort(buses.begin(), buses.end(), [&](json::Node& node1, json::Node& node2){
                    return node1.AsString() < node2.AsString();
                });
                answer["buses"] = std::move(buses);
            }
            else{
                answer["error_message"] = json::Node(std::string("not found"));
            }   
            answer["request_id"] = id;
            
            result.push_back(answer);
        }
        else if(i.AsDict().at("type").AsString() == "Bus"){
            auto node = i.AsDict();
            int id = node.at("id").AsInt(); 
            std::string name = node.at("name").AsString();

            json::Dict answer;
            

            if(request.GetCatalogue()->GetRoute(name) == nullptr){
                 answer["error_message"] = json::Node(std::string("not found"));
            }else{
                
                int route_length = request.GetRouteLength(name);
                double length = request.GetGeoRouteLength(name);
                int count = (int)request.GetCatalogue()->GetStops(name).size();
                if(request.GetCatalogue()->GetRouteType(name) == ctlg::BusRoute::Type::STRAIGHT){
                    count = count * 2 - 1;
                }
                
                answer["route_length"] = route_length;
                answer["curvature"] = (double)route_length / length;
                answer["unique_stop_count"] = (int)request.GetCatalogue()->GetUniqueStopsForRoute(name);
                answer["stop_count"] = count;
            }

            answer["request_id"] = id;
            result.push_back(answer);
        }
    }
    json::Print(json::Document(json::Node(result)), output);
}
