#include "stat_reader.h"
#include <iostream>
#include <iomanip>
#include <tuple>

namespace detail {

    using namespace std::literals;

    void StatReader::ParseAndPrintStat(const Catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
        std::ostream& output) {
        output << std::setprecision(6);

        if (request[0] == 'B') {

            auto bus = request.substr(request.find_first_of(' ') + 1);

            if (transport_catalogue.SearchRoute(bus)) {
                const auto info = transport_catalogue.GetRouteInfo(bus);
                output << request << ": "sv << info.stops_on_route << " stops on route, "sv << info.unique_stops << " unique stops, "sv << info.length << " route length"sv << std::endl;

            }
            else {
                output << request << ": not found"sv << std::endl;
            }
        }
        else if (request[0] == 'S') {
            auto stop = request.substr(request.find_first_of(' ') + 1);
            output << request << ": "sv;
            if (transport_catalogue.SearchStop(stop)) {
                const auto& info = transport_catalogue.GetStopInfo(stop);
                if (info.size()) {
                    output << "buses "sv;
                    for (auto i = info.begin(); i != info.end(); i++) {
                        output << *i << " "sv;
                    }
                    output << std::endl;
                }
                else {
                    output << "no buses"sv << std::endl;
                }
            }
            else {
                output << "not found"sv << std::endl;
            }
        }
    }

    void StatReader::Output(std::istream& cin, std::ostream& output, Catalogue::TransportCatalogue& catalogue) {
        int stat_request_count;
        cin >> stat_request_count >> std::ws;
        for (int i = 0; i < stat_request_count; ++i) {
            std::string line;
            getline(cin, line);
            ParseAndPrintStat(catalogue, line, output);
        }
    }
};