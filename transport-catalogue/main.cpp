#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    Catalogue::TransportCatalogue catalogue;

    detail::InputReader reader;
    reader.Input(cin, catalogue);
    detail::StatReader stat_reader;
    stat_reader.Output(cin, cout, catalogue);
}