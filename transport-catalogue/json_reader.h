#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <string_view>
#include <utility>
#include <vector>

namespace json_reader {

struct StatRequest {
	StatRequest(int id, std::string type)
		: id(id)
		, type(type) {
	}

	StatRequest(int id, std::string type, std::string name)
		: id(id)
		, type(type)
		, name(name) {
	}

	int id;
	std::string type;
	std::string name;
};

struct BusBuffer {
	std::string bus_name;
	std::vector<std::string> stops_names;
	bool is_round;
};

using DistancesBufferKey = std::pair<std::string, std::string>;

struct DistancesBufferHasher {
	std::size_t operator()(const DistancesBufferKey& key) const;
};

class JsonReader {
public:
	void ParseLine(std::istream& input);

	void FillDatabase(transport_catalogue::TransportCatalogue& database);

	void SetupMapRenderer(renderer::MapRenderer& map_renderer) const;

	void RequestAndPrint(RequestHandler& request_handler, std::ostream& out) const;

	std::vector<StatRequest> GetRequests() const;

	void Print(json::Document& doc, std::ostream& out) const;

private:
	void ParseStop(json::Dict dict);

	void ParseBus(json::Dict dict);

	std::vector<Stop> stops_buffer_;
	std::vector<BusBuffer> buses_buffer_;
	std::unordered_map<DistancesBufferKey, Distance, DistancesBufferHasher> stops_distances_buffer_;
	json::Array stat_requests_;
	json::Dict render_settings_;
};

} // namespace json_reader