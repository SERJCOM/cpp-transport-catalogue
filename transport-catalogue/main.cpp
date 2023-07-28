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


#include "log_duration.h"

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

    {
        LOG_DURATION("ParseData");

        reader.ParseData(handler);
    }

    reader.SetMapRenderer(map);

    LogDuration log_route("TransportRouter create");
    TransportRouter router(tr);
    log_route.Print();

    {
        LOG_DURATION("router init");
        router.InitRouter();
    }
    

    handler.SetRouter(router);


    {
        LOG_DURATION("PrintInformation");
        // reader.PrintInformation(std::cout, handler);
    }



}

int main(){

    Test1();


}