#include "map_renderer.h"
#include <iterator>


svg::Text ctlg::MapRenderer::CreateText(const ctlg::BusRoute* route,  int pos){
    svg::Text text1;
            
    text1.SetData(route->name); 
    text1.SetFillColor(data_.color_palette[pos % data_.color_palette.size() ]);
    text1.SetPosition(projector(route->buses[0]->coord));
    text1.SetOffset(svg::Point(data_.bus_label_offset.first, data_.bus_label_offset.second));
    text1.SetFontSize(data_.bus_label_font_size);
    text1.SetFontFamily("Verdana");
    text1.SetFontWeight("bold");

    return text1;
}

svg::Text ctlg::MapRenderer::CreatePodlozhka(const svg::Text &text)
{
    svg::Text podlozhka = text;
    podlozhka.SetFillColor(data_.underlayer_color);
    podlozhka.SetStrokeColor(data_.underlayer_color);
    podlozhka.SetStrokeWidth(data_.underlayer_width);
    podlozhka.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    podlozhka.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return podlozhka;
}

std::vector<const ctlg::BusStop*> ctlg::MapRenderer::CreateLines(const BusRoute *route, svg::Document &doc, int pos)
{
    svg::Polyline line;

    std::vector<const BusStop*> stops;
            
    for(const auto stop : route->buses){
        line.AddPoint(projector(stop->coord));
        stops.push_back(stop);
    }

    if(route->type == ctlg::BusRoute::Type::STRAIGHT){
        for(auto it = route->buses.end() - 2; it >= route->buses.begin(); it--){
            
            BusStop temp = *(*it);
            line.AddPoint(projector(temp.coord));
        }
    }
    line.SetStrokeColor(data_.color_palette[pos % data_.color_palette.size() ]);
    line.SetStrokeWidth(data_.line_width);
    line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    line.SetFillColor("none");


    doc.Add(line);


    return stops;
}

void ctlg::MapRenderer::DrawMap(std::ostream &out, const std::vector<const ctlg::BusRoute *> &routes)
{

    std::vector<geo::Coordinates> coords;

    for(const auto& route : routes){ 
        for(const auto stop : route->buses){
            coords.push_back(stop->coord);
        }
    }

    projector = SphereProjector(coords.begin(), coords.end(), data_.width, data_.height, data_.padding);

    svg::Document doc;

    int pos = 0;

    std::vector<const BusStop*> stops;
    {
        for(const auto& route : routes){ 
            auto lines = CreateLines(route, doc, pos);
            for(auto& line: lines){
                stops.push_back(std::move(line));
            }
            pos++;
        }   
    }   
    {    
        pos = 0;
        for(const auto& route : routes){

            auto text1 = CreateText(route, pos);
            auto podlozhka = CreatePodlozhka(text1);
            doc.Add(podlozhka);
            doc.Add(text1);
            
            if(route->type == ctlg::BusRoute::Type::STRAIGHT){
                size_t size = route->buses.size() - 1;
                if(route->buses[size]->coord != route->buses[0]->coord){
                    svg::Text text2 = text1;
                    text2.SetPosition(projector(route->buses[size]->coord));
                    podlozhka.SetPosition(projector(route->buses[size]->coord));
                    doc.Add(std::move(podlozhka));
                    doc.Add(std::move(text2));
                }
            }
            pos++;
        }
    }

    std::sort(stops.begin(), stops.end(), [&](const BusStop* stop1, const BusStop* stop2){
        return stop1->name < stop2->name;
    });

    std::vector<const BusStop*>::iterator it;

    while(true){
        it = std::unique(stops.begin(), stops.end(), [&](const BusStop* stop1, const BusStop* stop2){
            if(stop1->name == stop2->name && stop1->coord == stop2->coord){
                return true;
            }

            return false;
        });
        if(it == stops.end()){
            break;
        }
        stops.erase(it, stops.end());
    }

    { // circle
        pos = 0;
        
        for(const auto stop : stops){
            svg::Point point = projector(stop->coord);
            svg::Circle circle;
            circle.SetCenter(point);
            circle.SetRadius(data_.stop_radius);
            circle.SetFillColor("white");
            doc.Add(circle);
        }
    }
    {
        pos = 0;
        for(const auto stop : stops){
            svg::Text text;
            text.SetPosition(projector(stop->coord));
            text.SetOffset(svg::Point(data_.stop_label_offset.first, data_.stop_label_offset.second));
            text.SetFontSize(data_.stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name);
            text.SetFillColor("black");

            svg::Text podlozhka = CreatePodlozhka(text);

            doc.Add(std::move(podlozhka));
            doc.Add(std::move(text));
        }
    }

    doc.Render(out);
}



bool ctlg::IsZero(double value)
{
    return std::abs(value) < EPSILON;
}
