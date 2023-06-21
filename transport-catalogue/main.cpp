#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <sstream>
#include "json_reader.h"
#include "request_handler.h"
#include <fstream>
#include "map_renderer.h"
#include <algorithm>

using namespace std;
using namespace ctlg;
//using namespace detail;


void Test1(){
    TransportCatalogue tr;

    RequestHandler handler(&tr);

    JsonReader reader;

    MapRenderer map;

    handler.SetRenderMap(&map);

    reader.LoadDocument(std::cin);
    reader.ParseData(handler);
    reader.SetMapRenderer(map);

    reader.PrintInformation(std::cout, handler);
}

int main(){

    Test1();
   
}