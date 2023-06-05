#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"


namespace ctlg{

class JsonReader{
public:

    JsonReader() = default;

    void LoadDocument(std::istream& input);

    void SetRequest(Request& request);

    void GetInformation(std::ostream &output, Request &request);



private:
    json::Document doc_;

};

}