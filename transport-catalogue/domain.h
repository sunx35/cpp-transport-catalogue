#pragma once

#include "geo.h"

#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

 // мы намеренно не используем namespace, так как domain, это основные объекты, которые встречаются
 // во многих частях программы.

using Buses = std::set<std::string_view>;

using Distance = int;

struct Stop {
	std::string name;
	geo::Coordinates coordinates;
};

struct Bus {
	std::string name;
	std::vector<Stop*> stops;
	bool is_round = false;
};

struct BusResponse {
	bool bus_exist = false;
	size_t stops_count;
	size_t unique_stops_count;
	Distance route_length; // new route length, road-based
	double curvature;
};

struct StopResponse {
	bool stop_exist = false;
	std::vector<std::string_view> buses;
};

using BusesTable = std::unordered_map<std::string_view, Bus*>;