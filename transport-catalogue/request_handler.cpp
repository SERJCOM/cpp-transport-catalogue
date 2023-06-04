#include <utility>
#include <iostream>
#include "request_handler.h"
#include <iomanip>
#include <set>


using namespace ctlg;

// returning string, end of the string
std::string detail::SplitIntoWord::Split(char sym, int offset) {
    int begin = str.find_first_not_of(notof, end );
    int end = begin + 1;
    while(str[end] != sym && end != str.size()){
        end++;
    }
    if(end != str.size()){
        end += offset;
    }
    
    this->end = end;
    this->notof += sym;

    return str.substr(begin, end - begin );
}

std::string detail::SplitIntoWord::Split(char sym, std::string buffer, int offset){
    int begin = str.find_first_not_of(buffer, end );
    int end = begin + 1;
    while(str[end] != sym && end != str.size()){
        end++;
    }
    if(end != str.size()){
        end += offset;
    }
    
    this->end = end;

    return str.substr(begin, end - begin );
}

void ctlg::Request::AddNewNode(const std::string& str) {

    detail::SplitIntoWord temp(str);
    std::string command(temp.Split(' '));
    
    if(command == "Stop"){
        
        std::string name(temp.Split(':'));
        double first = stod(temp.Split(','));
        double second = stod(temp.Split(','));

        Coordinates temp_cord{first, second};
        catalogue_->AddBusStop(ctlg::BusStop(temp_cord, name));

        while(!temp.IsEnd()){
            int length = stoi(temp.Split('m'));
            temp.Split(' ');
            std::string new_name = temp.Split(',', " ");
            catalogue_->SetDistanceBetweenStops(name, new_name, length);
        }
    }

    if(command == "Bus"){

        std::string name = temp.Split(':');
        std::vector<std::string> buses;
        
        char typeSym = '>';
        ctlg::BusRoute::Type type = ctlg::BusRoute::Type::CYCLIC;

        if(str.find('-') != str.npos){
            typeSym = '-';
            type = ctlg::BusRoute::Type::STRAIGHT;
        }

        while(!temp.IsEnd()){
            buses.push_back(temp.Split(typeSym, -1));
        }


        catalogue_->AddBusRoute(std::move(buses), name, type);

    }
    
}

int ctlg::Request::GetRouteLength(std::string_view name) const
{
    int length = 0;

    auto stops = catalogue_->GetStops(name);

    for(auto it = stops.begin(); it != stops.end() - 1; it++){
        length += catalogue_->GetDistanceBetweenStops(it->name, (it + 1)->name);
    }

    if(catalogue_->GetRoute(name)->type == BusRoute::Type::CYCLIC){
        length *= 2;
    }

    return length;
}

double ctlg::Request::GetGeoRouteLength(std::string_view name) const
{
    double length = 0;

    auto stops = catalogue_->GetStops(name);

    for(auto it = stops.begin(); it != stops.end() - 1; it++){
        length += ctlg::ComputeDistance(it->coord, (it + 1)->coord);
    }

    if(catalogue_->GetRoute(name)->type == BusRoute::Type::CYCLIC){
        length *= 2;
    }

    return length;
    
}

TransportCatalogue *ctlg::Request::GetCatalogue() const
{
    return catalogue_.get();
}

// void ctlg::AddNewNode(std::istream &in, ctlg::TransportCatalogue &catalogue)
// {
//     std::string line;
//     std::getline(in, line);
//     int num = stoi(line);

//     for(int i = 0; i < num; i++){
//         std::getline(in, line);
//         ctlg::AddNewNode(line, catalogue);  
//     }

// }



void ctlg::GetRouteInfo(std::ostream &out, std::istream& in, const ctlg::TransportCatalogue &ctl){

    std::string request;

    std::getline(in, request);
    int num = std::stoi(request);

    for(int i = 0; i < num; i++){

        std::getline(in, request);

         detail::SplitIntoWord spl(request);
        std::string name = spl.Split(' ');
        
        if(name == "Bus"){

            out << "Bus ";
            std::string num = move(spl.Split('\n'));
            auto stops = ctl.GetStops(num);

            out << num << ": ";
            if(stops.size() == 0){
                out << "not found" << std::endl;    
                continue;
            }
            
            size_t numstops = stops.size();
            size_t unique = ctl.GetUniqueStopsForRoute(num);
            double length = 0;
            double distance = 0;

            for(auto it = stops.begin() + 1; it != stops.end(); it++ ){
                length += ComputeDistance((it - 1)->coord, it->coord);
                distance += ctl.GetDistanceBetweenStops((it - 1)->name, it->name);
            }

            if(ctl.GetRouteType(num) == ctlg::BusRoute::Type::STRAIGHT){
                numstops = numstops * 2 - 1;
                length *= 2;
                for(auto it = stops.end() - 1; it != stops.begin(); it--){
                    distance += ctl.GetDistanceBetweenStops(it->name, (it - 1)->name );
                }
            }

            out << numstops << " stops on route";
            out << ", " << unique << std::setprecision(6) << " unique stops, ";
            out << distance << std::setprecision(6) << " route length, ";
            out << distance / length << " curvature";
        } 
        else if(name == "Stop"){
            out << "Stop ";
            std::string name = spl.Split('\n');
            out << name << ":";
            auto route = ctl.GetRouteByStop(name);

            std::set<std::string_view> buses(route.begin(), route.end());

            if(buses.size() == 0){
                if(ctl.BusStopExist(name) == false){
                    out << " not found";
                }
                else{
                    out << " no buses";
                }
            }
            else{
                out << " buses";
                for(const auto& i : buses){
                    out << " " << i;
                }
            }
            
        }

        out << std::endl;
        
    }

    return;
}

void ctlg::Request::SetStop(const BusStop &stop)
{
    catalogue_->AddBusStop(stop);
}

void ctlg::Request::SetBus(const BusRoute &route, const std::vector<std::string> &stops)
{
    catalogue_->AddBusRoute(stops, route.name, route.type);
}

void ctlg::Request::SetDistance(std::string_view stop1, std::string_view stop2, double distance){
    catalogue_->SetDistanceBetweenStops(stop1, stop2, distance);
}




