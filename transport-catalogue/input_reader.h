#pragma once
#include "transport_catalogue.h"

#include <string_view>
#include <utility>
#include <vector>

namespace input_reader {

struct BusBuffer {
	std::string bus_name;
	std::vector<std::string> stops_names;
};

// нужен буфер для хранения расстояний между остановками (до того как имеем доступ ко всем указателям на остановки в deque)

class InputReader {

public:
	void ParseLine(std::string_view line);

	void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue);

private:
	void ParseStop(std::string_view line, size_t command_first_char);

	void ParseBus(std::string_view line, size_t command_first_char);

	std::vector<std::string> ParseStops(const std::string_view& line, size_t busname_end_pos);

	void FindAndAddAnotherStop(std::vector<std::string>& stops_middle_buffer, const std::string_view& line, size_t prev_stop_end_pos);

	void ParseStopsDistances(std::unordered_map<std::string, transport_catalogue::Distance>& distances_container, const std::string_view& line, size_t prev_end_pos);

	std::vector<transport_catalogue::Stop> stops_buffer_;
	std::vector<BusBuffer> buses_buffer_;
};

} // namespace input_reader