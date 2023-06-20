#include "json_reader.h"
#include <string>
#include <set>
#include "json_builder.h"


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

    json::Builder builder;

    builder.StartArray();

    for(auto i : stat_requests){
        if(i.AsDict().at("type").AsString() == "Stop"){
            auto node = i.AsDict();

            int id = node.at("id").AsInt();
            std::string name = node.at("name").AsString();

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
                builder.Key("error_message").Value(std::string("not found"));
            }   

            builder.Key("request_id").Value(id);
            
            builder.EndDict();
        }
        else if(i.AsDict().at("type").AsString() == "Bus"){
            auto node = i.AsDict();
            int id = node.at("id").AsInt(); 
            std::string name = node.at("name").AsString();

            builder.StartDict();

            if(request.GetCatalogue()->GetRoute(name) == nullptr){
                builder.Key("error_message").Value(std::string("not found"));
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
        else if(i.AsDict().at("type").AsString() == "Map"){
            auto node = i.AsDict();
            int id = node.at("id").AsInt(); 

            builder.StartDict();

            builder.Key("request_id").Value(id);

            std::string map = request.RenderMap();

            builder.Key("map").Value(map);

            builder.EndDict();

        }
    }

    builder.EndArray();
    json::Print(json::Document(builder.Build()), output);
}

void ctlg::JsonReader::SetMapRenderer(MapRenderer &render)
{
    auto render_settings = doc_.GetRoot().AsDict().at("render_settings").AsDict();


    render.width = render_settings.at("width").AsDouble();
    render.height = render_settings.at("height").AsDouble();
    render.padding = render_settings.at("padding").AsDouble();
    render.line_width = render_settings.at("line_width").AsDouble();
    render.stop_radius = render_settings.at("stop_radius").AsDouble();
    render.bus_label_font_size = render_settings.at("bus_label_font_size").AsDouble();

    auto vector = render_settings.at("bus_label_offset").AsArray();
    render.bus_label_offset.first = vector[0].AsDouble();
    render.bus_label_offset.second = vector[1].AsDouble();

    render.stop_label_font_size = render_settings.at("stop_label_font_size").AsDouble();

    vector = render_settings.at("stop_label_offset").AsArray();
    render.stop_label_offset.first = vector[0].AsDouble();
    render.stop_label_offset.second = vector[1].AsDouble();

    
    if(render_settings.at("underlayer_color").IsArray()){
        vector = render_settings.at("underlayer_color").AsArray();
        if(vector.size() == 4)
            render.underlayer_color = svg::Color(svg::Rgba(vector[0].AsInt(), vector[1].AsInt(), vector[2].AsInt(), vector[3].AsDouble()));
        else{
            render.underlayer_color = svg::Color(svg::Rgb(vector[0].AsInt(), vector[1].AsInt(), vector[2].AsInt()));
        }
    }
    else{
        render.underlayer_color = svg::Color(render_settings.at("underlayer_color").AsString());
    }

    render.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    for(const auto& i : render_settings.at("color_palette").AsArray() )
    if(i.IsString())
        render.color_palette.push_back(i.AsString());
    else{
        auto vector = i.AsArray();
        if(vector.size() == 4){
            render.color_palette.push_back(svg::Color(svg::Rgba(vector[0].AsInt(), vector[1].AsInt(), vector[2].AsInt(), vector[3].AsDouble())));
        }
        else{
            render.color_palette.push_back(svg::Color(svg::Rgb(vector[0].AsInt(), vector[1].AsInt(), vector[2].AsInt())));
        }
    }
    
}