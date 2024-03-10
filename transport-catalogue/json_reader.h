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
        void FillBaseRequests(const json::Dict& dict, Catalogue::TransportCatalogue& catalogue); //заполнение базовых запросов
        void RenderSettings(const json::Dict& dict, renderer::MapRenderer& renderer);
        void FillStatSettings(const json::Dict& dict); //заполнение запросов статистики



        void OutputJSON(std::ostream& output, const Catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& map);
    private:
        std::vector<StatRequests> stat_requests_;
    };
}