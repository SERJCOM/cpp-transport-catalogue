#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>
#include <string>

using namespace std;
using namespace ctlg;

int main(){
    TransportCatalogue tr;
   
    detail::AddNewNode(cin, tr);
    
    detail::GetRouteInfo(cout, cin, tr) ;
    
}