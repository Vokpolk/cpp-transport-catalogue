#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace detail {

    using namespace std::literals;
    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return { nan, nan };
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return { lat, lng };
    }

    std::vector<std::pair<std::string_view, double>> ParseStopAndDistance(std::string_view str) {
        std::vector<std::pair<std::string_view, double>> result;
        if (str.find(" to ") == str.npos) {
            return result;
        }
        //удаляем координаты из str
        str = str.substr(str.find_first_not_of(' '), str.size() - 1);
        str = str.substr(str.find_first_of(',') + 1, str.size() - 1);
        str = str.substr(str.find_first_of(',') + 1, str.size() - 1);

        while (!str.empty()) {
            str = str.substr(str.find_first_not_of(' '), str.size() - 1);

            std::pair<std::string_view, double> temp;
            temp.second = std::stod(std::string(str.substr(0, str.find_first_of('m'))));
            str = str.substr(str.find_first_of(' '), str.size() - 1);
            str = str.substr(str.find_first_not_of(' '), str.size() - 1);
            str = str.substr(str.find_first_of(' '), str.size() - 1);
            str = str.substr(str.find_first_not_of(' '), str.size() - 1);
            temp.first = str.substr(0, str.find_first_of(','));

            result.emplace_back(temp);

            if (str.find(" to ") == str.npos) {
                str = ""sv;
            }
            else {
                str = str.substr(str.find_first_of(','), str.size() - 1);
                str = str.substr(str.find_first_of(' '), str.size() - 1);
            }
        }
        return result;
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return { std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1)) };
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::ApplyCommands([[maybe_unused]] Catalogue::TransportCatalogue& catalogue) const {
        for (const auto& command : commands_) {
            if (command.command == "Stop"s) {
                catalogue.AddStop(command.id, ParseCoordinates(command.description).lat, ParseCoordinates(command.description).lng);
            }
            else if (command.command == "Bus"s) {
                catalogue.AddRoute(command.id, ParseRoute(command.description));
            }
        }


        for (const auto& command : commands_) {
            auto temp = ParseStopAndDistance(command.description);

            std::vector<std::pair<std::string_view, double>> stop_and_distance;
            for (const auto& s_and_d : temp) {
                stop_and_distance.emplace_back(s_and_d);
                catalogue.AddDistanceBetweenStops(catalogue.SearchStop(command.id), catalogue.SearchStop(s_and_d.first), s_and_d.second);
            }
        }
    }

    void InputReader::Input(std::istream& cin, Catalogue::TransportCatalogue& catalogue) {

        int base_request_count;
        cin >> base_request_count >> std::ws;
        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            getline(cin, line);
            ParseLine(line);
        }
        ApplyCommands(catalogue);
    }
};