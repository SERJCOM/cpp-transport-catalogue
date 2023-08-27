#pragma once

#include <string>
#include <fstream>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"



namespace serialize{

class Settings{
public:

    Settings() = default;

    Settings(std::string name);

    std::string_view GetFileName();

    void SetFileName(std::string name);
private:
std::string filename_;
};

class Serialization {
public:

    Serialization(Settings settings);

    bool Save();

    Serialization& CatalogueSerialization(const ctlg::TransportCatalogue& catalogue);

    Serialization& MapRendererSerialization(const ctlg::MapRenderer& renderer);

    // Serialization& RoutingSettingsSerialization(const ctlg::TransportRouter& router);

    void SetSettings(Settings settings);

private:
    srlz::TransportCatalogue db_;
    Settings settings_;
};

class Deserialization{
public:

    Deserialization(Settings settings);

    ctlg::TransportCatalogue CatalogueDeserialization();

    ctlg::MapRenderer MapRendererDeserialization();

    // ctlg::RouteSettings RoutingSettingsDeserialization();

private:
    srlz::TransportCatalogue db_;
    Settings settings_;
};


}