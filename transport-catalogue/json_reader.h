#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace json_reader {
    struct StatRequests {
        int id;
        const std::string type;
        const std::string name;
        const std::string from;
        const std::string to;
    };
    //struct StatRouteRequests {
    //    const std::string type;
    //    int id;
    //};

    class Reader {
    public:

        void InputJSON(std::istream& input
            , Catalogue::TransportCatalogue& catalogue
            , renderer::MapRenderer& renderer
            , Router::TransportRouter& router);
        void FillBaseRequests(const json::Dict& dict, Catalogue::TransportCatalogue& catalogue); //заполнение базовых запросов
        void RenderSettings(const json::Dict& dict, renderer::MapRenderer& renderer);
        void FillStatSettings(const json::Dict& dict); //заполнение запросов статистики
        void FillRouteSettings(
            const json::Dict& dict
            , Router::TransportRouter& router
            , Catalogue::TransportCatalogue& catalogue); //создание маршрута



        void OutputJSON(std::ostream& output
            , const Catalogue::TransportCatalogue& catalogue
            , renderer::MapRenderer& map
            , const Router::TransportRouter& router);
    private:
        std::vector<StatRequests> stat_requests_;
        //std::vector<StatRouteRequests> stat_route_requests_;

    };
}