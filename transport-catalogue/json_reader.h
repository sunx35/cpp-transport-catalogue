#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "transport_catalogue.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

#include <string_view>
#include <utility>
#include <vector>

namespace json_reader {

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
	void ReadInput(std::istream& input);
	transport_catalogue::TransportCatalogue CreateDatabase();
	renderer::MapRenderer CreateMapRenderer() const;
	router::TransportRouter CreateTransportRouter(const transport_catalogue::TransportCatalogue& catalogue) const;
	void RequestAndPrint(const RequestHandler& request_handler, std::ostream& out) const;

private:
	void ParseStop(json::Dict dict);
	void ParseBus(json::Dict dict);
	void Print(json::Document& doc, std::ostream& out) const;
	std::vector<svg::Color> MakeColorPalette(json::Array colors) const;
	void ProceedBusRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const;
	void ProceedStopRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const;
	void ProceedMapRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const;
	void ProceedRouteRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const;

	std::vector<Stop> stops_buffer_;
	std::vector<BusBuffer> buses_buffer_;
	std::unordered_map<DistancesBufferKey, Distance, DistancesBufferHasher> stops_distances_buffer_;
	json::Array stat_requests_;
	json::Dict render_settings_;
	json::Dict routing_settings_;
};

} // namespace json_reader