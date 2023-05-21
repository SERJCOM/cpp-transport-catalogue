#pragma once
#include <string>
#include "transport_catalogue.h"
#include <vector>
#include <iostream>

namespace ctlg{

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


    void AddNewNode(const std::string& str, ctlg::TransportCatalogue& catalogue) ;

    void AddNewNode(std::istream& in, ctlg::TransportCatalogue& catalogue) ;

}
}