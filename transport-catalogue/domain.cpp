#include "domain.h"
#include <memory>

ctlg::BusStop::BusStop(geo::Coordinates cord, std::string_view name)
{
    coord = cord;
    this->name = name;
}

ctlg::BusStop::BusStop(geo::Coordinates cord, std::string name)
{
    coord = cord;
    this->name = std::move(name);
}
