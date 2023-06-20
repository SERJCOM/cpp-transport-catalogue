#include "geo.h"

namespace geo{


bool Coordinates::operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
}

bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}

bool Coordinates::operator<(const Coordinates& other) const{
    return std::pair(lat, lng) < std::pair(other.lat, other.lng);
}   

bool Coordinates::operator>(const Coordinates& other) const{
    return std::pair(lat, lng) > std::pair(other.lat, other.lng);
}



}