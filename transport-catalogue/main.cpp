#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <sstream>
#include "json_reader.h"
#include "request_handler.h"

using namespace std;
using namespace ctlg;
using namespace detail;


void Test1(){
    TransportCatalogue tr;

    Request handler(&tr);

    JsonReader reader;
    reader.LoadDocument(std::cin);
    reader.SetRequest(handler);

    reader.GetInformation(std::cout, handler);

    //GetRouteInfo(std::cout, std::cin, tr);
}

int main(){

    Test1();
   
}