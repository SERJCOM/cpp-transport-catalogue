#include "serialization.h"
#include <unordered_map>
#include "map_renderer.h"
#include <iostream>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_set>

using namespace std;
using namespace ctlg;
using namespace serialize;


void serialize::Settings::SetFileName(string name){
    filename_ = move(name);
}

serialize::Settings::Settings(std::string name)
{
    SetFileName(move(name));
}

string_view serialize::Settings::GetFileName()
{
    return filename_;
}

serialize::Serialization::Serialization(Settings settings)
{
    SetSettings(move(settings));
}

bool serialize::Serialization::Save()
{
    ofstream file(settings_.GetFileName().data(), ios::binary);

    if(!db_.SerializeToOstream(&file)){
        return false;
    }

    return false;
}

Serialization &serialize::Serialization::CatalogueSerialization(const TransportCatalogue &catalogue)
{
    std::unordered_map<string, int> name_index; // stop_name --- index
    for(auto stop : catalogue.GetAllStops()){
        if(name_index.count(stop.name) == 0){
            name_index[stop.name] = name_index.size();
        }
    }

    for(const auto&[name, index] : name_index){
        auto stop = catalogue.GetStop(name);
        srlz::Stop _stop;
        _stop.set_name(index);
        _stop.set_latitude(stop->coord.lat);
        _stop.set_longitude(stop->coord.lng);
        *db_.add_stop() = _stop;
    }


    for(auto [stops, length] : catalogue.GetDistanceBetweenStops()){
        srlz::RoadDistances rdDist;
        string_view stop_from = stops.first->name;
        string_view stop_to = stops.second->name;
        rdDist.set_stop_from(name_index.at(stop_from.data()));
        rdDist.set_stop_to(name_index.at(stop_to.data()));

        rdDist.set_length(length);

        *db_.add_distance() = rdDist;
    }

    for(auto bus : catalogue.GetRouteDataBase()){
        srlz::Bus _bus;
        _bus.set_is_roundtrip((bus->type == ctlg::BusRoute::Type::CYCLIC)); // todo возможно ошибка
        _bus.set_name(bus->name);
        for(const BusStop* stop : bus->stops){
            _bus.add_stops(name_index.at(stop->name));
        }
        *db_.add_bus() = move(_bus);
    }

    vector<string> index_name(name_index.size());
    for(const auto& [name, index] : name_index){
        index_name[index] = name;
    }

    srlz::StopsDataBase stopsDb;
    for(const string& name : index_name){
        stopsDb.add_stops(name);
    }

    *db_.mutable_stops_db() = move(stopsDb);

    return *this;
}


srlz::Color GetColor(const svg::Color& color){
    srlz::Color _color;
    if(holds_alternative<string>(color)){
        srlz::ColorString colorstr;
        colorstr.set_color(std::get<string>(color));
        *_color.mutable_color_string() = move(colorstr);
        
    } else if(holds_alternative<svg::Rgb>(color)){
        srlz::ColorArray colorar;
        svg::Rgb rgb = get<svg::Rgb>(color);
        colorar.add_color(rgb.red);
        colorar.add_color(rgb.green);
        colorar.add_color(rgb.blue);
        *_color.mutable_color_array() = colorar;
    }
    else if(holds_alternative<svg::Rgba>(color)){
        srlz::ColorArray colorar;
        svg::Rgba rgb = get<svg::Rgba>(color);
        colorar.add_color(rgb.red);
        colorar.add_color(rgb.green);
        colorar.add_color(rgb.blue);
        colorar.add_color(rgb.opacity);
        *_color.mutable_color_array() = colorar;
    }

    return _color;
}


serialize::Serialization &serialize::Serialization::MapRendererSerialization(const MapRenderer &renderer)
{
    srlz::RenderSetting render;
    render.set_width(renderer.GetData().width);
    render.set_height(renderer.GetData().height);
    render.set_padding(renderer.GetData().padding);
    render.set_stop_radius(renderer.GetData().stop_radius);
    render.set_line_width(renderer.GetData().line_width);
    render.set_bus_label_font_size(renderer.GetData().bus_label_font_size);
    render.set_stop_label_font_size(renderer.GetData().stop_label_font_size);
    render.set_underlayer_width(renderer.GetData().underlayer_width);

    render.add_bus_label_offset(renderer.GetData().bus_label_offset.first);
    render.add_bus_label_offset(renderer.GetData().bus_label_offset.second);

    render.add_stop_label_offset(renderer.GetData().stop_label_offset.first);
    render.add_stop_label_offset(renderer.GetData().stop_label_offset.second);

    *render.mutable_underlayer_color() = GetColor(renderer.GetData().underlayer_color);

    for(const auto& color : renderer.GetData().color_palette){
        *render.add_color_palette() = GetColor(color);
    }

    *db_.mutable_render_settings() = move(render);

    return *this;

}

// Serialization &serialize::Serialization::RoutingSettingsSerialization(const ctlg::TransportRouter &router)
// {
//     srlz::RoutingSettings settings;
//     settings.set_bus_velocity(router.GetVelocity());
//     settings.set_bus_wait_time(router.GetVelocity());
//     *db_.mutable_routing_settings() = settings;
//     return *this;
// }

void serialize::Serialization::SetSettings(Settings settings)
{
    settings_ = move(settings);
}

serialize::Deserialization::Deserialization(Settings settings)
{
    ifstream file(settings.GetFileName().data(), ios::binary);

    db_.ParseFromIstream(&file);
}

ctlg::TransportCatalogue serialize::Deserialization::CatalogueDeserialization()
{
    TransportCatalogue catalogue;

    vector<string> stopsdb;
    stopsdb.reserve(db_.stops_db().stops_size());

    for(int i = 0; i < db_.stops_db().stops_size(); i++){
        stopsdb.push_back(db_.stops_db().stops(i));
    }

    for(int i = 0; i < db_.bus_size(); i++){
        ctlg::BusRoute bus;
        srlz::Bus prbus = db_.bus(i);

        bus.name = prbus.name();

        // if(bus.name == "g"){
        //     cout << "true" << endl;;
        // }

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

        catalogue.AddBusRoute(stops, std::move(bus.name), bus.type);
    }

    for(int i = 0; i < db_.stop_size(); i++){
        srlz::Stop prstop = db_.stop(i);
        ctlg::BusStop stop;

        stop.name = stopsdb[prstop.name()];
        stop.coord.lat = prstop.latitude();
        stop.coord.lng = prstop.longitude();
        catalogue.AddBusStop(stop);
    }

    for(int i = 0; i < db_.distance_size(); i++){
        srlz::RoadDistances distance = db_.distance(i);
        catalogue.SetDistanceBetweenStops(stopsdb.at(distance.stop_from()), stopsdb.at(distance.stop_to()), distance.length());
    }

    return catalogue;
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

ctlg::MapRenderer serialize::Deserialization::MapRendererDeserialization()
{
    MapRenderer render;

    
    render.GetData().width = db_.render_settings().width();
    render.GetData().height = db_.render_settings().height();
    render.GetData().padding = db_.render_settings().padding();
    render.GetData().line_width = db_.render_settings().line_width();
    render.GetData().stop_radius = db_.render_settings().stop_radius();
    render.GetData().bus_label_font_size = db_.render_settings().bus_label_font_size();
    int temp = db_.render_settings().bus_label_offset_size();
    render.GetData().bus_label_offset.first = db_.render_settings().bus_label_offset(0);
    render.GetData().bus_label_offset.second = db_.render_settings().bus_label_offset(1);
    render.GetData().stop_label_font_size = db_.render_settings().stop_label_font_size();
    render.GetData().stop_label_offset.first = db_.render_settings().stop_label_offset(0);
    render.GetData().stop_label_offset.second = db_.render_settings().stop_label_offset(1);
    
    render.GetData().underlayer_color = ParsingColor(db_.render_settings().underlayer_color());


    render.GetData().underlayer_width = db_.render_settings().underlayer_width();


    for(const srlz::Color& i : db_.render_settings().color_palette() )
    if(i.has_color_string())
        render.GetData().color_palette.push_back(i.color_string().color());
    else{
        render.GetData().color_palette.push_back(ParsingColor(i));
    }

    return render;
}

// ctlg::RouteSettings serialize::Deserialization::RoutingSettingsDeserialization()
// {
//     RouteSettings settings;
//     settings.velocity = db_.routing_settings().bus_velocity();
//     settings.wait = db_.routing_settings().bus_wait_time();

//     return settings;
// }

