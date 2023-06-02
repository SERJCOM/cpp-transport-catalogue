#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace ctlg;
using namespace detail;


void Test1(){
    TransportCatalogue tr;
   


    AddNewNode(std::cin, tr);

    GetRouteInfo(std::cout, std::cin, tr);
    

}

int main(){


Test1();
   
    
}