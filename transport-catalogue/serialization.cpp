#include "serialization.h"
#include <transport_catalogue.pb.h>
#include <unordered_map>

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

bool ctlg::Serialization(istream &in)
{
    Document doc = Load(in);
    string filename(GetFileName(doc));

    ofstream file(filename, ios::binary);  
    unordered_map<string, int> stop_index;
    srlz::TransportCatalogue db;

    Array base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

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

            //TODO добавить задание расстояния между остановками в две стороны если значения нет

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

    db.SerializeToOstream(&file);

    return true;
}

bool ctlg::Deserialization(const json::Document& doc, ctlg::RequestHandler handler)
{
    string filename(GetFileName(doc));

    ifstream file(filename, ios::binary);  

    srlz::TransportCatalogue db;

    if(!db.ParseFromIstream(&file))     return false;
    
    vector<string> stopsdb;
    stopsdb.reserve(db.mutable_stops_db()->stops_size());

    for(int i = 0; i < db.mutable_stops_db()->stops_size(); i++){
        stopsdb.push_back(db.mutable_stops_db()->stops(i));
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

    return true;
}

