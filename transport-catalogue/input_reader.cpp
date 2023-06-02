#include "input_reader.h"
#include <utility>
#include <iostream>


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

void detail::AddNewNode(const std::string& str, ctlg::TransportCatalogue& catalogue) {

    SplitIntoWord temp(str);
    std::string command(temp.Split(' '));
    
    if(command == "Stop"){
        
        std::string name(temp.Split(':'));
        double first = stod(temp.Split(','));
        double second = stod(temp.Split(','));

        Coordinates temp_cord{first, second};
        catalogue.AddBusStop(ctlg::BusStop(temp_cord, name));

        while(!temp.IsEnd()){
            int length = stoi(temp.Split('m'));
            temp.Split(' ');
            std::string new_name = temp.Split(',', " ");
            catalogue.SetDistanceBetweenStops(name, new_name, length);

            std::cout << "INFO: " << length << " " << name << " " << new_name << std::endl;
        }
    }

    if(command == "Bus"){

        // std::cout << "1 OK " << std::endl;
        std::string name = temp.Split(':');
        std::vector<std::string> buses;
        // std::cout << "2 OK " << std::endl;
        
        char typeSym = '>';
        ctlg::BusRoute::Type type = ctlg::BusRoute::Type::CYCLIC;

        if(str.find('-') != str.npos){
            typeSym = '-';
            type = ctlg::BusRoute::Type::STRAIGHT;
        }

        // std::cout << "3 OK " << std::endl;

        while(!temp.IsEnd()){
            buses.push_back(temp.Split(typeSym, -1));
        }

        // std::cout << "4 OK " << std::endl;

        catalogue.AddBusRoute(std::move(buses), name, type);

        // std::cout << "5 OK " << std::endl;
    }
    
}

void detail::AddNewNode(std::istream &in, ctlg::TransportCatalogue &catalogue)
{
    std::string line;
    std::getline(in, line);
    int num = stoi(line);

    for(int i = 0; i < num; i++){
        std::getline(in, line);
        detail::AddNewNode(line, catalogue);  
    }

}
