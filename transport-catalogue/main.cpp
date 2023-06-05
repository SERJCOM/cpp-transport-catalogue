#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <sstream>
#include "json_reader.h"
#include "request_handler.h"
#include <fstream>

using namespace std;
using namespace ctlg;
using namespace detail;


void Test1(){
    TransportCatalogue tr;

    Request handler(&tr);

    JsonReader reader;

    //std::ifstream in("input.txt"); 
    reader.LoadDocument(std::cin);
    reader.SetRequest(handler);

    reader.GetInformation(std::cout, handler);

}

int main(){

    Test1();
   
}