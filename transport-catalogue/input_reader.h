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

class InputReader {

public:
	void ParseLine(std::string_view line);

	void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue);

private:
	std::vector<std::string> ParseStops(const std::string_view& line, size_t busname_end_pos); // нужно ли использовать константную ссылку на string_view? есть ли смысл? учитывая, что string_view, это уже своего рода ссылка...

	void FindAndAddAnotherStop(std::vector<std::string>& stops_middle_buffer, const std::string_view& line, size_t prev_stop_end_pos);

	std::vector<transport_catalogue::Stop> stops_buffer_;
	std::vector<BusBuffer> buses_buffer_;
};

} // namespace input_reader