#pragma once

#include "transport_catalogue.h"
#include <memory>
#include <vector>
#include <iostream>

namespace ctlg{

// void AddNewNode(const std::string& str, ctlg::TransportCatalogue& catalogue);

// void AddNewNode(std::istream& in, ctlg::TransportCatalogue& catalogue);

void GetRouteInfo(std::ostream& out, std::istream& in, const ctlg::TransportCatalogue& ctl);

class Request{
public:
    Request() = default;

    // Request(std::shared_ptr<TransportCatalogue> catalogue):catalogue_(catalogue){}

    Request(TransportCatalogue* catalogue):catalogue_(catalogue){}

    void SetCatalogue(TransportCatalogue* catalogue){
        catalogue_ = catalogue;
    }

    void SetStop(const BusStop& stop);

    void SetBus(const BusRoute& route, const std::vector<std::string>& stops);

    void SetDistance(std::string_view stop1, std::string_view stop2, double distance);

    void AddNewNode(const std::string& str);

    int GetRouteLength(std::string_view name) const ;

    double GetGeoRouteLength(std::string_view name) const ;



    TransportCatalogue* GetCatalogue() const ;
    
private:

TransportCatalogue* catalogue_;

};



namespace detail{
class SplitIntoWord{
    public:

        SplitIntoWord(const std::string& str ): str(str) {};

        std::string Split(char sym, int offset = 0) ;

        std::string Split(char sym, std::string buffer, int offset = 0);

        bool IsEnd(){
            return (end ) == str.size();
        }

        void AddToBuffer(const std::string& str){
            notof += str;
            end += str.size();
        }

    private:
        int end = 0;
        const std::string& str;
        std::string notof = " ";
};
}

}
