#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace detail {

    void ParseAndPrintStat(const Catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output);

};