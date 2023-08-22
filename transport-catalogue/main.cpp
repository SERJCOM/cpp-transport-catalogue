#include <fstream>
#include <iostream>
#include <string_view>

#include "serialization.h"
#include "json_reader.h"
#include "transport_catalogue.h"

using namespace std::literals;
using namespace ctlg;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        Serialization(std::cin);

    } else if (mode == "process_requests"sv) {

        TransportCatalogue tr;
        RequestHandler handler(&tr);

        json::Document doc = json::Load(std::cin);

        Deserialization(doc, handler);

        PrintInformation(doc, std::cout, handler);

    } else {
        PrintUsage();
        return 1;
    }
}