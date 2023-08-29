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
     // stop_name --- index
    for(auto stop : catalogue.GetAllStops()){
        if(name_index_.count(stop.name) == 0){
            name_index_[stop.name] = name_index_.size();
        }
    }

    for(const auto&[name, index] : name_index_){
        auto stop = catalogue.GetStopByName(name);
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
        rdDist.set_stop_from(name_index_.at(stop_from.data()));
        rdDist.set_stop_to(name_index_.at(stop_to.data()));

        rdDist.set_length(length);

        *db_.add_distance() = rdDist;
    }

    for(auto bus : catalogue.GetRouteDataBase()){
        srlz::Bus _bus;
        _bus.set_is_roundtrip((bus->type == ctlg::BusRoute::Type::CYCLIC)); // todo возможно ошибка
        _bus.set_name(bus->name);
        for(const BusStop* stop : bus->stops){
            _bus.add_stops(name_index_.at(stop->name));
        }
        *db_.add_bus() = move(_bus);
    }

    vector<string> index_name(name_index_.size());
    for(const auto& [name, index] : name_index_){
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

void serialize::Serialization::SerializeGraph(const Graph & graph, srlz::RoutingSettings& settings)
{
    srlz::Graph _graph;
    // парсим ребра
    using Edge = graph::Edge<float>;
    for(int i = 0; i < graph.GetEdgeCount(); i++){
        Edge edge = graph.GetEdge(i);
        srlz::Edge _edge;
        _edge.set_from(edge.from);
        _edge.set_to(edge.to);

        if(edge.bus.size() > 0)
            _edge.set_bus(edge.bus.data());
        _edge.set_span(edge.span);
        _edge.set_weight(edge.weight);
        *_graph.add_edges() = _edge;
    }

    _graph.set_vertexsize(graph.GetVertexCount());

    *settings.mutable_graph() = move(_graph);
}

void serialize::Serialization::SerializeRouter(const ctlg::Router &router, srlz::RoutingSettings& settings)
{
    srlz::Router _router;

    for(const auto& array : router.GetRoutesInternalData()){
        srlz::RouteInternalDataArray dataArray;
        for(const auto& data : array){
            srlz::RouteInternalData _data;
            const ctlg::Router::RouteInternalData& info = data.value();
            _data.set_weight(info.weight);
            if(info.prev_edge.has_value())
                _data.mutable_prev_edge()->set_prev_edge(info.prev_edge.value());
            *dataArray.add_data() = _data;
        }
        *_router.add_routes_internal_data() = dataArray;
    }

    *settings.mutable_router() = _router;
}

Serialization &serialize::Serialization::RoutingSettingsSerialization(const ctlg::TransportRouter &router)
{
    srlz::RoutingSettings settings;
    settings.set_bus_velocity(router.GetVelocity());
    settings.set_bus_wait_time(router.GetVelocity());

    for(auto [name, pair_id] : router.GetStopsVertex()){
        srlz::StopNameVertexIds stopname;
        stopname.set_name(name_index_.at(name.data()));
        stopname.set_vertex1(pair_id.first.value());
        stopname.set_vertex2(pair_id.second.value());
        *settings.add_stop_name_vertex_id() = stopname;
    }

    for(auto [id, name] : router.GetVertexWaitStopsName()){
        srlz::VertexIdStopName vertexname;
        vertexname.set_name(name_index_.at(name.data()));
        vertexname.set_vertex(id);
        *settings.add_vertexidwait_stopname() = vertexname;
    }

    for(auto [id, name] : router.GetVertexRideStopsName()){
        srlz::VertexIdStopName vertexname;
        vertexname.set_name(name_index_.at(name.data()));
        vertexname.set_vertex(id);
        *settings.add_vertexidride_stopname() = vertexname;
    }

    SerializeGraph(router.GetGraph(), settings);
    SerializeRouter(router.GetRouter(), settings);

    *db_.mutable_routing_settings() = settings;
    return *this;
}

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

    stopsdb_.reserve(db_.stops_db().stops_size());

    for(int i = 0; i < db_.stops_db().stops_size(); i++){
        stopsdb_.push_back(db_.stops_db().stops(i));
    }

    for(int i = 0; i < db_.bus_size(); i++){
        ctlg::BusRoute bus;
        srlz::Bus prbus = db_.bus(i);

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
            stops.push_back(stopsdb_.at(prbus.stops(i)));
        }

        catalogue.AddBusRoute(stops, std::move(bus.name), bus.type);
    }

    for(int i = 0; i < db_.stop_size(); i++){
        srlz::Stop prstop = db_.stop(i);
        ctlg::BusStop stop;

        stop.name = stopsdb_[prstop.name()];
        stop.coord.lat = prstop.latitude();
        stop.coord.lng = prstop.longitude();
        catalogue.AddBusStop(stop);
    }

    for(int i = 0; i < db_.distance_size(); i++){
        srlz::RoadDistances distance = db_.distance(i);
        catalogue.SetDistanceBetweenStops(stopsdb_.at(distance.stop_from()), stopsdb_.at(distance.stop_to()), distance.length());
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


ctlg::TransportRouter serialize::Deserialization::RoutingSettingsDeserialization(const ctlg::TransportCatalogue& catalogue)
{
    srlz::RoutingSettings _settings = db_.routing_settings();
    TransportRouter router;

    for(int i = 0; i <_settings.stop_name_vertex_id_size(); i++){
        srlz::StopNameVertexIds name_ids = _settings.stop_name_vertex_id(i) ;
        int name_id = name_ids.name();
        const ctlg::BusStop* stopptr = catalogue.GetStopByName(stopsdb_.at(name_id));
        string_view name(stopptr->name);
        router.stopname_vertexid_[name] = std::pair{name_ids.vertex1(), name_ids.vertex2()}; 
    }

    for(int i = 0; i < _settings.vertexidride_stopname_size(); i++){
        srlz::VertexIdStopName ride_name = _settings.vertexidride_stopname(i);
        int name_id = ride_name.name();
        const ctlg::BusStop* stopptr = catalogue.GetStopByName(stopsdb_.at(name_id));
        string_view name(stopptr->name);
        router.vertexidride_stopname_[ride_name.vertex()] = name;
    }

    for(int i = 0; i < _settings.vertexidwait_stopname_size(); i++){
        srlz::VertexIdStopName wait_name = _settings.vertexidwait_stopname(i);
        int name_id = wait_name.name();
        const ctlg::BusStop* stopptr = catalogue.GetStopByName(stopsdb_.at(name_id));
        string_view name(stopptr->name);
        router.vertexidwait_stopname_[wait_name.vertex()] = name;
    }

    router.GetGraph() = DeserializeGraph(catalogue);
    router.InitRouter();
    router.GetRouter().SetGraph(router.GetGraph());
    DeserializeRouter(router.GetRouter());

    return router;        
}

ctlg::Graph serialize::Deserialization::DeserializeGraph(const ctlg::TransportCatalogue& catalogue)
{
    
    srlz::Graph graph = db_.routing_settings().graph();
    ctlg::Graph _graph;
    _graph.SetVertexCount(graph.vertexsize());

    for(int i = 0; i < graph.edges_size(); i++){
        srlz::Edge edge = graph.edges(i);
        ctlg::TransportRouter::Edge _edge;
        _edge.from = edge.from();
        _edge.to = edge.to();
        _edge.span = edge.span();
        _edge.weight = edge.weight();
        if(edge.bus().size() > 0)
            _edge.bus = catalogue.GetRouteByName(edge.bus())->name;
        _graph.AddEdge(_edge);
    }

    return _graph;
}

void serialize::Deserialization::DeserializeRouter(ctlg::Router& router)
{
    ctlg::Router::RoutesInternalData internalData;
    const srlz::Router _router = db_.routing_settings().router();

    for(int i = 0; i < _router.routes_internal_data_size(); i++){
        const srlz::RouteInternalDataArray& array = _router.routes_internal_data(i);        
        std::vector<std::optional<ctlg::Router::RouteInternalData>> temp;
        for(int j = 0; j < array.data_size(); j++){
            const srlz::RouteInternalData& data = array.data(j);
            ctlg::Router::RouteInternalData _data;
            if(data.has_prev_edge())
                _data.prev_edge = data.prev_edge().prev_edge();
            _data.weight = data.weight();
            temp.push_back(_data);
        }
        internalData.push_back(temp);
    }

    router.SetRoutesInternalData(move(internalData));
}
