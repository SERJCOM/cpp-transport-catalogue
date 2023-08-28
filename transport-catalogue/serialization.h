#pragma once

#include <string>
#include <fstream>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
#include <unordered_map>
#include <vector>


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

    Serialization& RoutingSettingsSerialization(const ctlg::TransportRouter& router);

    void SetSettings(Settings settings);

private:

    void SerializeGraph(const ctlg::Graph& graph, srlz::RoutingSettings& settings);

    void SerializeRouter(const ctlg::Router& router, srlz::RoutingSettings& settings);

    int GetStringIndex(std::string_view str) const {
        return name_index_.at(str.data());
    }

    srlz::TransportCatalogue db_;
    Settings settings_;
    std::unordered_map<std::string, int> name_index_;
};

class Deserialization{
public:

    Deserialization(Settings settings);

    ctlg::TransportCatalogue CatalogueDeserialization();

    ctlg::MapRenderer MapRendererDeserialization();

    ctlg::TransportRouter RoutingSettingsDeserialization(const ctlg::TransportCatalogue& catalogue);

private:

    ctlg::Graph DeserializeGraph(const ctlg::TransportCatalogue& catalogue);

    void DeserializeRouter(ctlg::Router& router);

    srlz::TransportCatalogue db_;
    Settings settings_;
    std::vector<std::string> stopsdb_;
};


}