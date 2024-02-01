#pragma once

#include <iosfwd>
#include <string_view>
#include <iostream>
#include <string>
#include "transport_catalogue.h"

namespace detail {
    class StatReader {
    public:
        void ParseAndPrintStat(const Catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void Output(std::istream& cin, std::ostream& output, Catalogue::TransportCatalogue& catalogue);
    };
};