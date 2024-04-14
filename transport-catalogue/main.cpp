#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include <iostream>

int main() {
    using namespace std;

    Router::TransportRouter router;

    Catalogue::TransportCatalogue catalogue;
    auto json_reader = json_reader::Reader();
    auto map_renderer = renderer::MapRenderer();
    json_reader.InputJSON(cin, catalogue, map_renderer, router);
    json_reader.OutputJSON(cout, catalogue, map_renderer, router);

    return 0;
}