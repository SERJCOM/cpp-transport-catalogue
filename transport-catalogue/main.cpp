#include <fstream>
#include <iostream>
#include <string_view>

#include "serialization.h"
#include "json_reader.h"
#include "transport_catalogue.h"

using namespace std::literals;
using namespace ctlg;
using namespace std;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

// int test1(){
//     ifstream file("test1.txt");
//     TransportCatalogue catalogue;
//     MapRenderer renderer;
//     // TransportRouter router(catalogue);

//     RequestHandler handler(catalogue);
//     handler.SetRenderMap(renderer);
//     // handler.SetRouter(router);

//     JsonReader reader;
//     reader.LoadDocument(file);
//     reader.ParseData(handler);
//     reader.ParseMapRenderer(renderer);
//     // reader.ParseRoutingSettings(router);

//     serialize::Settings settings(reader.GetFileName());
//     serialize::Serialization serialization(settings);
//     serialization.CatalogueSerialization(catalogue);
//     serialization.MapRendererSerialization(renderer);
//     // serialization.RoutingSettingsSerialization(router);
//     serialization.Save();
// }

// int test2(){

//     ifstream file("test2.txt");

//     JsonReader reader;
//     reader.LoadDocument(file);
//     serialize::Settings settings(reader.GetFileName());

//     serialize::Deserialization deser(settings);
//     TransportCatalogue catalogue(deser.CatalogueDeserialization());
//     MapRenderer renderer(deser.MapRendererDeserialization());
//     // TransportRouter router(catalogue, deser.RoutingSettingsDeserialization());
//     // router.InitRouter();

//     RequestHandler handler(catalogue);
//     handler.SetRenderMap(renderer);
//     // handler.SetRouter(router);


//     reader.PrintInformation(std::cout, handler);
// }


// int test3(){
//     ifstream file("test3.txt");
//     TransportCatalogue catalogue;
//     MapRenderer renderer;
//     // TransportRouter router(catalogue);

//     RequestHandler handler(catalogue);
//     handler.SetRenderMap(renderer);
//     // handler.SetRouter(router);

//     JsonReader reader;
//     reader.LoadDocument(file);
//     reader.ParseData(handler);
//     reader.ParseMapRenderer(renderer);
//     // reader.ParseRoutingSettings(router);

//     reader.PrintInformation(std::cout, handler);
// }

// int main(){
//     test2();
// }


int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        TransportCatalogue catalogue;
        MapRenderer renderer;
        // TransportRouter router(catalogue);

        RequestHandler handler(catalogue);
        handler.SetRenderMap(renderer);
        // handler.SetRouter(router);

        JsonReader reader;
        reader.LoadDocument(std::cin);
        reader.ParseData(handler);
        reader.ParseMapRenderer(renderer);
        // reader.ParseRoutingSettings(router);

        serialize::Settings settings(reader.GetFileName());
        serialize::Serialization serialization(settings);
        serialization.CatalogueSerialization(catalogue);
        serialization.MapRendererSerialization(renderer);
        // serialization.RoutingSettingsSerialization(router);
        serialization.Save();

    } else if (mode == "process_requests"sv) {

        
        JsonReader reader;
        reader.LoadDocument(std::cin);
        serialize::Settings settings(reader.GetFileName());

        serialize::Deserialization deser(settings);
        TransportCatalogue catalogue(deser.CatalogueDeserialization());
        MapRenderer renderer(deser.MapRendererDeserialization());
        // TransportRouter router(catalogue, deser.RoutingSettingsDeserialization());
        // router.InitRouter();

        RequestHandler handler(catalogue);
        handler.SetRenderMap(renderer);
        // handler.SetRouter(router);


        reader.PrintInformation(std::cout, handler);


    } else {
        PrintUsage();
        return 1;
    }
}   