#pragma once
#include "geo.h"

#include "domain.h"

#include <deque>
#include <set>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport_catalogue {

using DistancesKey = std::pair<Stop*, Stop*>;

struct DistancesHasher {
	std::size_t operator()(const DistancesKey& key) const;
};

class TransportCatalogue {
public:
	void AddBus(Bus&& bus);

	void AddStop(Stop&& stop);

	Bus* FindBusByName(std::string_view name) const;

	Stop* FindStopByName(std::string_view name) const;

	BusResponse GetBusInfo(std::string_view busname) const;

	StopResponse GetStopInfo(std::string_view stopname) const;

	BusesTable GetAllBuses() const;

	void AddDistance(Stop* stop1, Stop* stop2, Distance distance);

	Distance GetDistance(Stop* stop1, Stop* stop2) const;

private:
	double ComputeRouteLength(std::string_view busname) const;

	Distance ComputeRoadBasedRouteLength(std::string_view busname) const;

	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Stop*> stops_table_;
	std::unordered_map<std::string_view, Bus*> buses_table_;
	std::unordered_map<std::string_view, Buses> stops_to_buses_;
	std::unordered_map<DistancesKey, Distance, DistancesHasher> distances_;
};

} // namespace transport_catalogue