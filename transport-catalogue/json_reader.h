#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <string>


namespace ctlg{

class JsonReader{
public:

    JsonReader() = default;

    void LoadDocument(std::istream& input);

    void ParseData(RequestHandler& request);

    void PrintInformation(std::ostream &output, RequestHandler &request);

    void ParseMapRenderer(MapRenderer& render);

    void ParseRoutingSettings(TransportRouter& router);

    std::string GetFileName();

private:
    json::Document doc_;

};

void PrintInformation(const json::Document& doc, std::ostream &output, RequestHandler &request);


}