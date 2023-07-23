#include "json_reader.h"
#include <string>
#include <set>
#include "json_builder.h"
#include <string_view>
#include <iostream>

using namespace ctlg;
using namespace std;

void ctlg::JsonReader::LoadDocument(std::istream &input)
{
    doc_ = json::Load(input);
}

void ctlg::JsonReader::ParseData(RequestHandler &request)
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

    auto routing_settings = doc_.GetRoot().AsDict().at("routing_settings").AsDict();
        
    int bus_wait_time = routing_settings.at("bus_wait_time").AsInt();
    float bus_velocity = routing_settings.at("bus_velocity").AsInt();

    request.SetVelocity(bus_velocity);
    request.SetWaitTime(bus_wait_time);
        
    

}


void ctlg::JsonReader::PrintInformation(std::ostream &output, RequestHandler &request){


    auto stat_requests = doc_.GetRoot().AsDict().at("stat_requests").AsArray();

    json::Builder builder;

    builder.StartArray();

    for(const auto& node : stat_requests){

        if(node.AsDict().at("type").AsString() == "Stop"){

            
            auto node_dict = node.AsDict();

            int id = node_dict.at("id").AsInt();
            std::string name = node_dict.at("name").AsString();
            
            builder.StartDict();
            

            if(request.GetCatalogue()->BusStopExist(name)){
               json::Array buses ;
               const auto& set = request.GetCatalogue()->GetRouteByStop(name);
                for(const auto& str : set){
                    buses.push_back(std::string(str));
                }
                std::sort(buses.begin(), buses.end(), [&](json::Node& node1, json::Node& node2){
                    return node1.AsString() < node2.AsString();
                });

                builder.Key("buses").Value(std::move(buses));
            }
            else{
                builder.Key("error_message").Value("not found");
            }   
            builder.Key("request_id").Value(id);

            
            builder.EndDict();
        }
        else if(node.AsDict().at("type").AsString() == "Bus"){


            auto node_dict = node.AsDict();
            int id = node_dict.at("id").AsInt(); 
            std::string name = node_dict.at("name").AsString();

            builder.StartDict();
            

            if(request.GetCatalogue()->GetRoute(name) == nullptr){
                builder.Key("error_message").Value("not found");
            }else{
                
                int route_length = request.GetRouteLength(name);
                double length = request.GetGeoRouteLength(name);
                int count = (int)request.GetCatalogue()->GetStops(name).size();
                if(request.GetCatalogue()->GetRouteType(name) == ctlg::BusRoute::Type::STRAIGHT){
                    count = count * 2 - 1;
                }
                
                builder.Key("route_length").Value(route_length);
                builder.Key("curvature").Value((double)route_length / length);
                builder.Key("unique_stop_count").Value((int)request.GetCatalogue()->GetUniqueStopsForRoute(name));
                builder.Key("stop_count").Value(count);

            }

            builder.Key("request_id").Value(id);
            builder.EndDict();
        }
        else if(node.AsDict().at("type").AsString() == "Map"){
            auto node_dict = node.AsDict();
            int id = node_dict.at("id").AsInt(); 

            builder.StartDict();

            builder.Key("request_id").Value(id);

            std::string map = request.RenderMap();

            builder.Key("map").Value(std::move(map));

            builder.EndDict();
        }
        else if(node.AsDict().at("type").AsString() == "Route"){

            auto node_dict = node.AsDict();
            int id = node_dict.at("id").AsInt();

            string_view from = node_dict.at("from").AsString();
            string_view to = node_dict.at("to").AsString();

            builder.StartDict();

            float total_time = 0;


            auto routes = request.GetRoute(from, to);

            if(routes.has_value()){

            builder.Key("items").StartArray();


            for(const auto& route : routes.value()){


                builder.StartDict();
                

                if(holds_alternative<RouteWait>(route)){

                    RouteWait wait = std::get<RouteWait>(route);

                    builder.Key("stop_name").Value(std::string(wait.name));
                    builder.Key("time").Value(wait.time);
                    total_time += wait.time;
                    builder.Key("type").Value("Wait");

                } else if(holds_alternative<RouteBus>(route)){

                    RouteBus bus = std::get<RouteBus>(route);

                    builder.Key("bus").Value(std::string(bus.name));
                    builder.Key("span_count").Value(bus.span_count);
                    builder.Key("time").Value(bus.time);
                    total_time += bus.time;
                    builder.Key("type").Value("Bus");
                }

                builder.EndDict();
            }

            builder.EndArray();

            builder.Key("total_time");
            builder.Value(total_time);

            }
            else{
                builder.Key("error_message").Value("not found");
            }
            
            
            builder.Key("request_id").Value(id);

            builder.EndDict();
        }
    }

    builder.EndArray();

    json::Print(json::Document(std::move(builder.Build())), output);
}

svg::Color ParsingColor(const json::Node& node){


    if(node.IsArray()){
        auto array = node.AsArray();

        if(array.size() == 4){
            return svg::Color(svg::Rgba(array[0].AsInt(), array[1].AsInt(), array[2].AsInt(), array[3].AsDouble()));
        }

        return svg::Color(svg::Rgb(array[0].AsInt(), array[1].AsInt(), array[2].AsInt()));
    }
    else{
        return svg::Color(node.AsString());
    }
    
}


void ctlg::JsonReader::SetMapRenderer(MapRenderer &render)
{
    auto render_settings = doc_.GetRoot().AsDict().at("render_settings").AsDict();

    MapRenderData data;


    data.width = render_settings.at("width").AsDouble();
    data.height = render_settings.at("height").AsDouble();
    data.padding = render_settings.at("padding").AsDouble();
    data.line_width = render_settings.at("line_width").AsDouble();
    data.stop_radius = render_settings.at("stop_radius").AsDouble();
    data.bus_label_font_size = render_settings.at("bus_label_font_size").AsDouble();

    auto vector = render_settings.at("bus_label_offset").AsArray();
    data.bus_label_offset.first = vector[0].AsDouble();
    data.bus_label_offset.second = vector[1].AsDouble();

    data.stop_label_font_size = render_settings.at("stop_label_font_size").AsDouble();

    vector = render_settings.at("stop_label_offset").AsArray();
    data.stop_label_offset.first = vector[0].AsDouble();
    data.stop_label_offset.second = vector[1].AsDouble();

    
    data.underlayer_color = ParsingColor(render_settings.at("underlayer_color"));


    data.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    for(const auto& i : render_settings.at("color_palette").AsArray() )
    if(i.IsString())
        data.color_palette.push_back(i.AsString());
    else{
        auto vector = i.AsArray();
        data.color_palette.push_back(ParsingColor(vector));
    }

    render.data_ = data;
    
}