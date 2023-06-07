#include "map_renderer.h"

void ctlg::MapRenderer::DrawMap(std::ostream &out, const std::vector<const ctlg::BusRoute *> &routes)
{

    std::vector<geo::Coordinates> coords;

    for(const auto& route : routes){ 
        for(const auto stop : route->buses){
            coords.push_back(stop->coord);
        }
    }

    projector = SphereProjector(coords.begin(), coords.end(), width, height, padding);

    svg::Document doc;

    int pos = 0;

    std::vector<const BusStop*> stops;
    {
    for(const auto& route : routes){ 
        svg::Polyline line;
        
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
        line.SetStrokeColor(color_palette[pos % color_palette.size() ]);
        line.SetStrokeWidth(line_width);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetFillColor("none");
        pos++;

        doc.Add(line);
    }   
    }
    {    
    pos = 0;
    for(const auto& route : routes){
        svg::Text text1;
        
        text1.SetData(route->name); 
        text1.SetFillColor(color_palette[pos % color_palette.size() ]);
        text1.SetPosition(projector(route->buses[0]->coord));
        text1.SetOffset(svg::Point(bus_label_offset.first, bus_label_offset.second));
        text1.SetFontSize(bus_label_font_size);
        text1.SetFontFamily("Verdana");
        text1.SetFontWeight("bold");

        svg::Text podlozhka = text1;
        podlozhka.SetFillColor(underlayer_color);
        podlozhka.SetStrokeColor(underlayer_color);
        podlozhka.SetStrokeWidth(underlayer_width);
        podlozhka.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        podlozhka.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        doc.Add(podlozhka);
        doc.Add(text1);
        
        if(route->type == ctlg::BusRoute::Type::STRAIGHT){
            size_t size = route->buses.size() - 1;
            if(route->buses[size]->coord != route->buses[0]->coord){
                svg::Text text2 = text1;
                text2.SetPosition(projector(route->buses[size]->coord));
                podlozhka.SetPosition(projector(route->buses[size]->coord));
                doc.Add(podlozhka);
                doc.Add(text2);
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
            circle.SetRadius(stop_radius);
            circle.SetFillColor("white");
            doc.Add(circle);
            
        }
    }
    {
        pos = 0;

        for(const auto stop : stops){
            svg::Text text;
            text.SetPosition(projector(stop->coord));
            text.SetOffset(svg::Point(stop_label_offset.first, stop_label_offset.second));
            text.SetFontSize(stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name);
            text.SetFillColor("black");

            svg::Text podlozhka = text;
            podlozhka.SetFillColor(underlayer_color);
            podlozhka.SetStrokeColor(underlayer_color);
            podlozhka.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            podlozhka.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            podlozhka.SetStrokeWidth(underlayer_width);

            doc.Add(podlozhka);
            doc.Add(text);
        }
        
    }

    doc.Render(out);
}

bool ctlg::IsZero(double value)
{
    return std::abs(value) < EPSILON;
}
