#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"


namespace ctlg{

class JsonReader{
public:

    JsonReader() = default;

    void LoadDocument(std::istream& input);

    void ParseData(RequestHandler& request);

    void PrintInformation(std::ostream &output, RequestHandler &request);

    void SetMapRenderer(MapRenderer& render);



private:
    json::Document doc_;

};

}