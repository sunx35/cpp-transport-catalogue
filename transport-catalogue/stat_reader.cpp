#include "stat_reader.h"

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& catalogue, std::string_view line, std::ostream& out) {
	using namespace std::literals;
	// Bus 750
	// Stop Marushkino

	if (line.at(line.find_first_not_of(' ')) == 'B') {
		size_t busname_begin_pos = line.find_first_not_of(' ', 3);
		size_t busname_end_pos = line.find_last_not_of(' ') + 1;
		size_t busname_size = busname_end_pos - busname_begin_pos;
		std::string_view busname = line.substr(busname_begin_pos, busname_size);

		try {
			transport_catalogue::BusResponse data = catalogue.GetBusInfo(busname);

			out << "Bus "s + std::string(busname) + ": " + std::to_string(data.stops_count) + " stops on route, "
				+ std::to_string(data.unique_stops_count) + " unique stops, "
				+ std::to_string(data.route_length) + " route length" << std::endl;
		}
		catch (const std::invalid_argument& e) {
			out << "Bus "s + std::string(busname) + ": not found"s << std::endl;
			return;
		}
	}
	else {
		size_t stopname_begin_pos = line.find_first_not_of(' ', 4);
		size_t stopname_end_pos = line.find_last_not_of(' ') + 1;
		size_t stopname_size = stopname_end_pos - stopname_begin_pos;
		std::string_view stopname = line.substr(stopname_begin_pos, stopname_size);

		// Stop X: buses bus1 bus2 ... busN

		transport_catalogue::StopResponse response = catalogue.GetStopInfo(stopname);

		if (!response.stop_exist) {
			out << "Stop "s << stopname << ": not found"s << std::endl;
		}
		else if (response.stop_exist && response.buses.empty()) {
			out << "Stop "s << stopname << ": no buses"s << std::endl;
		}
		else if (response.stop_exist && !response.buses.empty()) {
			std::string bus_line;
			for (const auto& bus : response.buses) {
				bus_line.append(bus);
				bus_line.push_back(' ');
			}
			out << "Stop "s << stopname << ": buses "s << bus_line << std::endl;
		}
	}
}