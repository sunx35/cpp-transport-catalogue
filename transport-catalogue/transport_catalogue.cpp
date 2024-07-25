#include "transport_catalogue.h"

namespace transport_catalogue {

// добавление маршрута в базу
void TransportCatalogue::AddBus(Bus&& bus) {
	buses_.push_back(std::move(bus));
	buses_table_.insert({ buses_.back().name, &buses_.back() });
	for (const auto& stop : buses_.back().stops) {
		stops_to_buses_.at(stop->name).insert(buses_.back().name);
	}
}

// добавление остановки в базу
void TransportCatalogue::AddStop(Stop&& stop) {
	stops_.push_back(std::move(stop));
	stops_table_.insert({ stops_.back().name, &stops_.back() });
	stops_to_buses_[stops_.back().name];
}

// поиск маршрута по имени
Bus* TransportCatalogue::FindBusByName(std::string_view name) const {
	if (!buses_table_.count(name)) {
		throw std::invalid_argument("Incorrect name");
	}
	return buses_table_.at(name);
}

// поиск остановки по имени
Stop* TransportCatalogue::FindStopByName(std::string_view name) const {
	if (!stops_table_.count(name)) {
		throw std::invalid_argument("Incorrect name");
	}
	return stops_table_.at(name);
}

// получение информации о маршруте
BusResponse TransportCatalogue::GetBusInfo(std::string_view busname) const {
	BusResponse response;
	try {
		const auto& stops = FindBusByName(busname)->stops; // может кинуть исключение
		response.bus_exist = true;
		response.stops_count = stops.size();
		std::unordered_set<Stop*> unique_stops;
		for (const auto& stop : stops) {
			unique_stops.insert(stop);
		}
		response.unique_stops_count = unique_stops.size();
		auto straight_route_length = ComputeRouteLength(busname);
		response.route_length = ComputeRoadBasedRouteLength(busname);
		response.curvature = response.route_length / straight_route_length;

		return response;
	}
	catch (std::invalid_argument&) {
		response.bus_exist = false;
		return response;
	}
}

StopResponse TransportCatalogue::GetStopInfo(std::string_view stopname) const {
	StopResponse response;

	if (!stops_to_buses_.count(stopname)) {
		response.stop_exist = false;
		return response;
	}
	if (stops_to_buses_.count(stopname) && stops_to_buses_.at(stopname).empty()) {
		response.stop_exist = true;
		return response;
	}
	if (stops_to_buses_.count(stopname) && !stops_to_buses_.at(stopname).empty()) {
		response.stop_exist = true;
		for (const auto& bus : stops_to_buses_.at(stopname)) {
			response.buses.push_back(bus);
		}
		return response;
	}

	return response;
}

BusesTable TransportCatalogue::GetAllBuses() const {
	return buses_table_;
}

void TransportCatalogue::AddDistance(Stop* stop1, Stop* stop2, Distance distance) {
	distances_.insert({ { stop1, stop2 }, distance });
}

Distance TransportCatalogue::GetDistance(Stop* stop1, Stop* stop2) const {
	if (!distances_.count({ stop1, stop2 })) {
		return distances_.at({ stop2, stop1 });
	}
	return distances_.at({ stop1, stop2 });
}

double TransportCatalogue::ComputeRouteLength(std::string_view busname) const {
	double overall_length = 0;
	for (size_t i = 1; i < FindBusByName(busname)->stops.size(); ++i) {
		overall_length += geo::ComputeDistance(FindBusByName(busname)->stops[i]->coordinates, FindBusByName(busname)->stops[i - 1]->coordinates);
	}
	return overall_length;
}

Distance TransportCatalogue::ComputeRoadBasedRouteLength(std::string_view busname) const {
	Distance overall_length = 0;
	const auto& stops = FindBusByName(busname)->stops;
	for (size_t i = 1; i < stops.size(); ++i) {
		overall_length += GetDistance(stops[i - 1], stops[i]);
	}
	return overall_length;
}

bool operator==(const DistancesKey& lhs, const DistancesKey& rhs) {
	return lhs.first == rhs.first && lhs.second == rhs.second;
}

std::size_t DistancesHasher::operator()(const DistancesKey& key) const {
	return std::hash<const void*>()(key.first) + 37 * std::hash<const void*>()(key.second);
}

} // namespace transport_catalogue