#include "serialization.h"
#include <transport_catalogue.pb.h>
#include <unordered_map>
#include "map_renderer.h"
#include <iostream>

using namespace json;
using namespace std;
using namespace ctlg;


std::string_view GetFileName(const json::Document &doc)
{
    return doc.GetRoot().AsDict().at("serialization_settings").AsDict().at("file").AsString();   
}

int GetStopIndex(unordered_map<string, int>& stop_index, const string& name){
    if(stop_index.count(name) == 0){
        stop_index[name] = stop_index.size();
    }

    return stop_index.at(name);
}

void BaseRequestsSerialization(srlz::TransportCatalogue& db, const Document& doc){
    Array base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();
    unordered_map<string, int> stop_index;

    for(int i = 0; i < base_requests.size(); i++){
        Dict data = base_requests[i].AsDict();

        if(data["type"].AsString() == "Bus"){
            srlz::Bus bus;

            bus.set_name(data["name"].AsString());
            bus.set_is_roundtrip(data["is_roundtrip"].AsBool());
            
            for(const auto& node : data["stops"].AsArray()){
                const string& stop = node.AsString();
                int index = GetStopIndex(stop_index, stop);
                bus.add_stops(index);
            }

            *db.add_bus() = std::move(bus);
        }
        else{
            srlz::Stop stop;

            stop.set_latitude(data["latitude"].AsDouble());
            stop.set_longitude(data["longitude"].AsDouble());
            stop.set_name(data["name"].AsString());

            for(const auto& node : data["road_distances"].AsDict()){
                srlz::RoadDistances rd;
                rd.set_stop(GetStopIndex(stop_index, node.first));
                rd.set_length(node.second.AsInt());
                *stop.add_road_distances() = std::move(rd);
            }

            *db.add_stop() = std::move(stop);
        }
    }

    std::vector<string> dbstops(stop_index.size());
    for(const auto&[stop, index] : stop_index){
        dbstops[index] = stop;
    }

    srlz::StopsDataBase stopdatabase;
    for(auto& stop : dbstops){
        stopdatabase.add_stops(std::move(stop));
    }

    *db.mutable_stops_db() = move(stopdatabase);
}


srlz::Color ParseColor(const Node& node){
    using namespace srlz;
    Color color;
    if(node.IsString()){
        ColorString colorString;
        colorString.set_color(node.AsString());
        *color.mutable_color_string() = move(colorString);
    }
    else {
        bool temp = node.IsArray();
        // cout << temp << endl;
        ColorArray colorArray;
        for(const Node& colorNode : node.AsArray()){
            colorArray.add_color(colorNode.AsDouble()) ;
        }
        *color.mutable_color_array() = move(colorArray);
    }

    return color;
}

void RenderSettingsSerialization(srlz::TransportCatalogue& db,  const Document& doc){
    using namespace srlz;
    RenderSetting render;
    Dict setting = doc.GetRoot().AsDict().at("render_settings").AsDict();
    render.set_width(setting.at("width").AsDouble());
    render.set_height(setting.at("height").AsDouble());
    render.set_padding(setting.at("padding").AsDouble());
    render.set_stop_radius(setting.at("stop_radius").AsDouble());
    render.set_line_width(setting.at("line_width").AsDouble());
    render.set_bus_label_font_size(setting.at("bus_label_font_size").AsDouble());
    render.set_stop_label_font_size(setting.at("stop_label_font_size").AsDouble());
    render.set_underlayer_width(setting.at("underlayer_width").AsDouble());

    for(const Node& node : setting.at("bus_label_offset").AsArray()){
        render.add_bus_label_offset(node.AsDouble());
    }
    int temp = render.bus_label_offset_size();

    for(const Node& node : setting.at("stop_label_offset").AsArray()){
        render.add_stop_label_offset(node.AsDouble());
    }


    *render.mutable_underlayer_color() = ParseColor(setting.at("underlayer_color"));

    for(const Node& node : setting.at("color_palette").AsArray()){
        *render.add_color_palette() = ParseColor(node);
    }

    *db.mutable_render_settings() = move(render);
}

void RoutingSettingsSerialization(srlz::TransportCatalogue& db,  const Document& doc){
    using namespace srlz;
    RoutingSettings routing;
    Dict settings = doc.GetRoot().AsDict().at("routing_settings").AsDict();
    routing.set_bus_velocity(settings.at("bus_velocity").AsDouble());
    routing.set_bus_wait_time(settings.at("bus_wait_time").AsDouble());

    *db.mutable_routing_settings() = routing;
}



bool ctlg::Serialization(istream &in)
{
    Document doc = Load(in);
    string filename(GetFileName(doc));

    ofstream file(filename, ios::binary);  
    
    srlz::TransportCatalogue db;

    BaseRequestsSerialization(db, doc);
    RenderSettingsSerialization(db, doc);
    RoutingSettingsSerialization(db, doc);

    db.SerializeToOstream(&file);

    return true;
}


void BaseRequestsDeserialization(ctlg::RequestHandler& handler, const srlz::TransportCatalogue& db){
    vector<string> stopsdb;
    stopsdb.reserve(db.stops_db().stops_size());

    for(int i = 0; i < db.stops_db().stops_size(); i++){
        stopsdb.push_back(db.stops_db().stops(i));
    }

    for(int i = 0; i < db.bus_size(); i++){
        ctlg::BusRoute bus;
        srlz::Bus prbus = db.bus(i);

        bus.name = prbus.name();
        if(prbus.is_roundtrip()){
            bus.type = ctlg::BusRoute::Type::CYCLIC;
        }
        else{
            bus.type = ctlg::BusRoute::Type::STRAIGHT;
        }

        vector<string> stops;
        stops.reserve(prbus.stops_size());
        for(int i = 0; i < prbus.stops_size(); i++){
            stops.push_back(stopsdb.at(prbus.stops(i)));
        }
        handler.GetCatalogue()->AddBusRoute(stops, std::move(bus.name), bus.type);
    }

    for(int i = 0; i < db.stop_size(); i++){
        srlz::Stop prstop = db.stop(i);
        ctlg::BusStop stop;

        stop.name = prstop.name();
        stop.coord.lat = prstop.latitude();
        stop.coord.lng = prstop.longitude();
        handler.GetCatalogue()->AddBusStop(stop);

        for(int i = 0; i < prstop.road_distances_size(); i++){
            handler.GetCatalogue()->SetDistanceBetweenStops(stop.name, stopsdb.at(prstop.mutable_road_distances(i)->stop()), prstop.mutable_road_distances(i)->length());
        }
    }
}

svg::Color ParsingColor(const srlz::Color& color){

    if(color.has_color_array()){
        srlz::ColorArray colorArray = color.color_array();
        if(colorArray.color_size() == 4){
            return svg::Color(svg::Rgba(colorArray.color(0), colorArray.color(1), colorArray.color(2), colorArray.color(3)));
        }

        return svg::Color(svg::Rgb(colorArray.color(0), colorArray.color(1), colorArray.color(2)));
    }
    else{
        return svg::Color(color.color_string().color());
    }
}

void RenderSettingsDeserialization(MapRenderer &render, const srlz::RenderSetting& render_settings){
    using namespace srlz;
    MapRenderData data;

    data.width = render_settings.width();
    data.height = render_settings.height();
    data.padding = render_settings.padding();
    data.line_width = render_settings.line_width();
    data.stop_radius = render_settings.stop_radius();
    data.bus_label_font_size = render_settings.bus_label_font_size();
    int temp = render_settings.bus_label_offset_size();
    data.bus_label_offset.first = render_settings.bus_label_offset(0);
    data.bus_label_offset.second = render_settings.bus_label_offset(1);
    data.stop_label_font_size = render_settings.stop_label_font_size();
    data.stop_label_offset.first = render_settings.stop_label_offset(0);
    data.stop_label_offset.second = render_settings.stop_label_offset(1);
    
    data.underlayer_color = ParsingColor(render_settings.underlayer_color());


    data.underlayer_width = render_settings.underlayer_width();


    for(const Color& i : render_settings.color_palette() )
    if(i.has_color_string())
        data.color_palette.push_back(i.color_string().color());
    else{
        data.color_palette.push_back(ParsingColor(i));
    }
    //TODO убрать поле data_ и сделать нормальный сеттер и геттер
    render.data_ = move(data);
}

void RoutingSettingsDeserialization(ctlg::RequestHandler& handler, srlz::RoutingSettings settings){
    handler.SetVelocity(settings.bus_velocity());
    handler.SetWaitTime(settings.bus_wait_time());
}

bool ctlg::Deserialization(const json::Document& doc, ctlg::RequestHandler& handler){
    string filename(GetFileName(doc));

    ifstream file(filename, ios::binary);  

    srlz::TransportCatalogue db;

    if(!db.ParseFromIstream(&file))     return false;
    
    BaseRequestsDeserialization(handler, db);
    RenderSettingsDeserialization(*handler.GetRenderMap(), db.render_settings());
    RoutingSettingsDeserialization(handler, db.routing_settings());

    return true;
}

