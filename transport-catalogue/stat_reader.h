#pragma once

#include "input_reader.h"
#include <string>

namespace ctlg{

namespace detail{

    //void GetRouteInfo(const std::string& request, const ctlg::TransportCatalogue& ctl) ;

    void GetRouteInfo(std::ostream& out, std::istream& in, const ctlg::TransportCatalogue& ctl) ;

}

}