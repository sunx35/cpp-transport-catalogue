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

using DistancesBufferKey = std::pair<std::string, std::string>;

struct DistancesBufferHasher {
	std::size_t operator()(const DistancesBufferKey& key) const;
};

class InputReader {
public:
	void ParseLine(std::string_view line);

	void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue);

private:
	void ParseStop(std::string_view line, size_t command_first_char);

	void ParseBus(std::string_view line, size_t command_first_char);

	std::vector<std::string> ParseStops(const std::string_view& line, size_t busname_end_pos);

	void FindAndAddAnotherStop(std::vector<std::string>& stops_middle_buffer, const std::string_view& line, size_t prev_stop_end_pos);

	void ParseStopsDistances(const std::string_view& stop_from, const std::string_view& line, size_t prev_end_pos);

	std::vector<transport_catalogue::Stop> stops_buffer_;
	std::vector<BusBuffer> buses_buffer_;
	std::unordered_map<DistancesBufferKey, transport_catalogue::Distance, DistancesBufferHasher> stops_distances_buffer_;
};

} // namespace input_reader