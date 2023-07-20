#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <sstream>
#include "json_reader.h"
#include "request_handler.h"
#include <fstream>
#include "map_renderer.h"
#include <algorithm>
#include "router.h"
#include <fstream>

using namespace std;
using namespace ctlg;
using namespace graph;


void Test1(){

    ifstream file("vivod.txt");

    TransportCatalogue tr;

    RequestHandler handler(&tr);

    JsonReader reader;

    MapRenderer map;

    handler.SetRenderMap(&map);
    reader.LoadDocument(file);
    reader.ParseData(handler);

    std::cout << "START TEST" << std::endl;

    reader.SetMapRenderer(map);

    std::cout << "MAP TEST" << std::endl;


    TransportRouter router(tr);

    std::cout << "create TEST" << std::endl;

    router.InitRouter();

    std::cout << "init TEST" << std::endl;

    handler.SetRouter(router);

    std::cout << "set TEST" << std::endl;

    reader.PrintInformation(std::cout, handler);

    std::cout << "END TEST" << std::endl;

}

int main(){

    Test1();
   
}