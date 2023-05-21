#include "stat_reader.h"
#include <iomanip>
#include <iostream>
#include <set>

using namespace std;

using namespace ctlg;

void detail::GetRouteInfo(std::ostream &out, std::istream& in, const ctlg::TransportCatalogue &ctl){

    string request;

    getline(in, request);
    int num = stoi(request);

    for(int i = 0; i < num; i++){

        getline(in, request);

        SplitIntoWord spl(request);
        string name = spl.Split(' ');
        
        if(name == "Bus"){

            out << "Bus ";
            string num = move(spl.Split('\n'));
            auto stops = ctl.GetStops(num);

            out << num << ": ";
            if(stops.size() == 0){
                out << "not found" << endl;    
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
            out << ", " << unique << setprecision(6) << " unique stops, ";
            out << distance << setprecision(6) << " route length, ";
            out << distance / length << " curvature";
        } 
        else if(name == "Stop"){
            out << "Stop ";
            string name = spl.Split('\n');
            out << name << ":";
            auto route = ctl.GetRouteByStop(name);

            set<string_view> buses(route.begin(), route.end());

            if(buses.size() == 0){
                if(ctl.IsBusStop(name) == false){
                    out << " not found";
                }
                else{
                    out << " no buses";
                }
            }
            else{
                out << " buses";
                for(const auto& i : buses){
                    cout << " " << i;
                }
            }
            
        }

        out << endl;
        
    }

    return;
}


