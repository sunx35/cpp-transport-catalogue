#pragma once
#include "geo.h"

#include <deque>
#include <set>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport_catalogue {

struct Stop {
	std::string name;
	geo::Coordinates coordinates;
};

struct Bus {
	std::string name;
	std::vector<Stop*> stops;
};

struct BusResponse {
	size_t stops_count;
	size_t unique_stops_count;
	double route_length;
};

struct StopResponse {
	bool stop_exist;
	std::vector<std::string_view> buses;
};

class TransportCatalogue {

public:
	void AddBus(Bus&& bus);

	void AddStop(Stop&& stop);

	Bus* FindBusByName(std::string_view name) const;

	Stop* FindStopByName(std::string_view name) const;

	BusResponse GetBusInfo(std::string_view busname) const;

	StopResponse GetStopInfo(std::string_view stopname) const;

private:
	double ComputeRouteLength(std::string_view busname) const;

	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Stop*> stops_table_;
	std::unordered_map<std::string_view, Bus*> buses_table_;
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses_;
};

} // namespace transport_catalogue