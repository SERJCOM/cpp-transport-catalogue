#include "json_reader.h"
#include <string>

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

            std::cout << "1 BUS" << std::endl;
           std::string name = nodedict.at("name").AsString();
          std::cout << "2 BUS" << std::endl;
            
           std::vector<std::string> stops;
           if(nodedict.at("is_roundtrip").AsBool()){
            bus.type = ctlg::BusRoute::Type::CYCLIC;
            std::cout << "3 BUS" << std::endl;
           }

            
           else{
           std::cout << "31 BUS" << std::endl;
            bus.type = ctlg::BusRoute::Type::STRAIGHT;
           }
            std::cout << "4 BUS" << std::endl;
           
           for(auto i : nodedict.at("stops").AsArray()){
                stops.push_back(i.AsString());
           }
            std::cout << "5 BUS" << std::endl;
            
           request.SetBus(bus, stops);
           std::cout << "6 BUS" << std::endl;
        }
        else if(nodedict.at("type").AsString() == "Stop"){
            std::string name = nodedict.at("name").AsString();
            std::cout << "name " << name << std::endl;
            double latitude = nodedict.at("latitude").AsDouble();
            double longitude = nodedict.at("longitude").AsDouble();
            std::cout << latitude << " " << longitude << std::endl;
            json::Dict distance = nodedict.at("road_distances").AsDict();
            BusStop stop({latitude, longitude}, std::move(name));
            request.GetCatalogue()->AddBusStop(stop);
            std::cout << "1 STOP" << std::endl;
            std::cout << (bool)(request.GetCatalogue()->GetStop("name") == nullptr) <<  "bool" << std::endl;
            std::cout << "2 STOP" << std::endl;
            for(auto i : distance){
                request.GetCatalogue()->SetDistanceBetweenStops(name, i.first, i.second.AsDouble()) ;
               
            }

            std::cout << "3 STOP" << std::endl;
        }
    }
    

}

void ctlg::JsonReader::GetInformation(std::ostream &output, Request &request){

    json::Array output_d;

    auto stat_requests = doc_.GetRoot().AsDict().at("stat_requests").AsArray();

    std::cout << "1 OK" << std::endl;

    for(auto i : stat_requests){
        if(i.AsDict().at("type").AsString() == "Stop"){

            json::Dict answer;

            auto node = i.AsDict();
            int id = node.at("id").AsInt();
            
            std::string name = node.at("name").AsString();
            std::cout << "2 OK" << std::endl;
            if(request.GetCatalogue()->BusStopExist(name)){
                std::cout << "3 OK" << std::endl;
               json::Array buses ;

               const auto& set = request.GetCatalogue()->GetRouteByStop(name);

                std::cout << "31 OK" << std::endl;

               std::cout << set.size() << std::endl;

                std::cout << "32 OK" << std::endl;
                for(const auto& str : set){

                    std::cout << str << std::endl;
                    buses.push_back(std::string(str));
                }


                answer["buses"] = std::move(buses);
                std::cout << "4 OK" << std::endl;
            }
            else{
                std::cout << "5 OK" << std::endl;
                answer["error_message"] = "not found";
            }

            std::cout << "6 OK" << std::endl;
            answer["request_id"] = id;

            json::Array temp;
            temp.push_back(answer);

            json::Print(json::Document(json::Node(temp)), output);

            std::cout << "7 OK" << std::endl;
            
        }
        else if(i.AsDict().at("Bus").AsString() == "Bus"){
            json::Dict answer;

            auto node = i.AsDict();

            int id = node.at("id").AsInt(); 

            std::string name = node.at("name").AsString();

            int route_length = request.GetRouteLength(name);
            double length = request.GetGeoRouteLength(name);

            answer["route_length"] = route_length;
            answer["curvature"] = (double)route_length / length;
            answer["unique_stop_count"] = (int)request.GetCatalogue()->GetUniqueStopsForRoute(name);
            answer["stop_count"] = (int)request.GetCatalogue()->GetStops(name).size();
            answer["request_id"] = id;

            json::Print(json::Document(answer), output);

        }
    }

}
