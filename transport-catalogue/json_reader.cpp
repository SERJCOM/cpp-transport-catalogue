#include "json_reader.h"
#include <string>
#include <set>


using namespace ctlg;

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
    

}

void ctlg::JsonReader::PrintInformation(std::ostream &output, RequestHandler &request){

    auto stat_requests = doc_.GetRoot().AsDict().at("stat_requests").AsArray();

    json::Array result;


    for(const auto& node : stat_requests){
        if(node.AsDict().at("type").AsString() == "Stop"){
            auto node_dict = node.AsDict();

            int id = node_dict.at("id").AsInt();
            std::string name = node_dict.at("name").AsString();
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
        else if(node.AsDict().at("type").AsString() == "Bus"){
            auto node_dict = node.AsDict();
            int id = node_dict.at("id").AsInt(); 
            std::string name = node_dict.at("name").AsString();

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
        else if(node.AsDict().at("type").AsString() == "Map"){
            auto node_dict = node.AsDict();
            int id = node_dict.at("id").AsInt(); 

            json::Dict answer;

            answer["request_id"] = id;

            std::string map = request.RenderMap();

            answer["map"] = map;

            result.push_back(answer);
        }
    }
    json::Print(json::Document(json::Node(result)), output);
}

svg::Color ParsingColor(const json::Array& array){
    if(array.size() == 4){
        return svg::Color(svg::Rgba(array[0].AsInt(), array[1].AsInt(), array[2].AsInt(), array[3].AsDouble()));
    }

    return svg::Color(svg::Rgb(array[0].AsInt(), array[1].AsInt(), array[2].AsInt()));
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

    
    if(render_settings.at("underlayer_color").IsArray()){
        vector = render_settings.at("underlayer_color").AsArray();
        data.underlayer_color = ParsingColor(vector);
    }
    else{
        data.underlayer_color = svg::Color(render_settings.at("underlayer_color").AsString());
    }

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