#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace json_reader {
    struct StatRequests {
        int id;
        const std::string type;
        const std::string name;
    };

    class Reader {
    public:

        void InputJSON(std::istream& input, Catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer);
        void RenderSettings(const json::Dict& dict, renderer::MapRenderer& renderer);

        void OutputJSON(std::ostream& output, const Catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& map);
    private:
        std::vector<StatRequests> stat_requests_;
    };
}