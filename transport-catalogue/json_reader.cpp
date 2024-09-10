#include "json_reader.h"

#include "request_handler.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

using namespace std::literals;

void JsonReader::ReadInput(std::istream& input) {
	json::Document doc = json::Load(input);

	json::Array base_requests = doc.GetRoot().AsMap().at("base_requests").AsArray();
	render_settings_ = doc.GetRoot().AsMap().at("render_settings").AsMap();
	stat_requests_ = doc.GetRoot().AsMap().at("stat_requests").AsArray();
	routing_settings_ = doc.GetRoot().AsMap().at("routing_settings").AsMap();

	for (const auto& req : base_requests) {
		if (req.AsMap().at("type").AsString() == "Stop") {
			ParseStop(req.AsMap());
		}
		else {
			ParseBus(req.AsMap());
		}
	}
}

transport_catalogue::TransportCatalogue JsonReader::CreateDatabase() {
	transport_catalogue::TransportCatalogue database;

	// добавить все остановки
	for (Stop& stop : stops_buffer_) {
		database.AddStop(std::move(stop));
	}

	// добавить все расстояния между остановками
	for (const auto& [pair_stops_names, distance] : stops_distances_buffer_) {
		Stop* stop1 = database.FindStopByName(pair_stops_names.first);
		Stop* stop2 = database.FindStopByName(pair_stops_names.second);
		database.AddDistance(stop1, stop2, distance);
	}

	// добавить все автобусы
	for (auto& bus_buffer : buses_buffer_) {
		Bus bus;
		bus.name = bus_buffer.bus_name;
		for (auto& stopname : bus_buffer.stops_names) {
			bus.stops.push_back(database.FindStopByName(stopname));
		}
		bus.is_round = bus_buffer.is_round;
		database.AddBus(std::move(bus));
	}

	return database;
}

renderer::MapRenderer JsonReader::CreateMapRenderer() const {
	renderer::MapRenderer map_renderer;

	map_renderer.SetWidth(render_settings_.at("width").AsDouble());
	map_renderer.SetHeight(render_settings_.at("height").AsDouble());
	map_renderer.SetPadding(render_settings_.at("padding").AsDouble());
	map_renderer.SetStopRadius(render_settings_.at("stop_radius").AsDouble());
	map_renderer.SetLineWidth(render_settings_.at("line_width").AsDouble());
	map_renderer.SetBusLabelFontSize(render_settings_.at("bus_label_font_size").AsInt());

	svg::Point bus_label_offset{
		render_settings_.at("bus_label_offset").AsArray().at(0).AsDouble(),
		render_settings_.at("bus_label_offset").AsArray().at(1).AsDouble()
	};
	map_renderer.SetBusLabelOffset(bus_label_offset);

	map_renderer.SetStopLabelFontSize(render_settings_.at("stop_label_font_size").AsInt());

	svg::Point stop_label_offset{
		render_settings_.at("stop_label_offset").AsArray().at(0).AsDouble(),
		render_settings_.at("stop_label_offset").AsArray().at(1).AsDouble()
	};
	map_renderer.SetStopLabelOffset(stop_label_offset);

	json::Node underlayer_color_node = render_settings_.at("underlayer_color");

	if (underlayer_color_node.IsString()) {
		map_renderer.SetUnderlayerColor(underlayer_color_node.AsString());
	}
	else if (underlayer_color_node.IsArray() && underlayer_color_node.AsArray().size() == 3) {
		map_renderer.SetUnderlayerColor(svg::Rgb{
			static_cast<uint8_t>(underlayer_color_node.AsArray().at(0).AsInt()),
			static_cast<uint8_t>(underlayer_color_node.AsArray().at(1).AsInt()),
			static_cast<uint8_t>(underlayer_color_node.AsArray().at(2).AsInt())
			});
	}
	else if (underlayer_color_node.IsArray() && underlayer_color_node.AsArray().size() == 4) {
		map_renderer.SetUnderlayerColor(svg::Rgba{
			static_cast<uint8_t>(underlayer_color_node.AsArray().at(0).AsInt()),
			static_cast<uint8_t>(underlayer_color_node.AsArray().at(1).AsInt()),
			static_cast<uint8_t>(underlayer_color_node.AsArray().at(2).AsInt()),
			underlayer_color_node.AsArray().at(3).AsDouble()
			});
	}

	map_renderer.SetUnderlayerWidth(render_settings_.at("underlayer_width").AsDouble());

	json::Array colors = render_settings_.at("color_palette").AsArray();

	map_renderer.SetColorPalette(MakeColorPalette(colors));

	return map_renderer;
}

std::vector<svg::Color> JsonReader::MakeColorPalette(json::Array colors) const {
	std::vector<svg::Color> color_palette;

	for (const auto& color : colors) {
		if (color.IsString()) {
			color_palette.push_back(color.AsString());
		}
		else if (color.IsArray() && color.AsArray().size() == 3) {
			color_palette.emplace_back(svg::Rgb{
				static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
				static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
				static_cast<uint8_t>(color.AsArray().at(2).AsInt())
				});
		}
		else if (color.IsArray() && color.AsArray().size() == 4) {
			color_palette.emplace_back(svg::Rgba{
				static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
				static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
				static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
				color.AsArray().at(3).AsDouble()
				});
		}
	}

	return color_palette;
}

router::TransportRouter JsonReader::CreateTransportRouter(const transport_catalogue::TransportCatalogue& catalogue) const {
	RoutingSettings settings;
	settings.bus_wait_time = routing_settings_.at("bus_wait_time"s).AsInt();
	settings.bus_velocity = routing_settings_.at("bus_velocity"s).AsDouble();

	return router::TransportRouter{ settings, catalogue };
}

void JsonReader::ProceedBusRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const {
	// запрос информации об автобусе:
	BusResponse response = request_handler.GetBusInfo(request.AsMap().at("name").AsString());

	if (response.bus_exist == false) {
		responses.StartDict()
			.Key("error_message"s).Value("not found"s)
			.Key("request_id"s).Value(request.AsMap().at("id").AsInt())
			.EndDict();
	}
	else {
		responses.StartDict()
			.Key("curvature"s).Value(response.curvature)
			.Key("request_id"s).Value(request.AsMap().at("id").AsInt())
			.Key("route_length"s).Value(response.route_length)
			.Key("stop_count"s).Value(static_cast<int>(response.stops_count))
			.Key("unique_stop_count"s).Value(static_cast<int>(response.unique_stops_count))
			.EndDict();
	}
}

void JsonReader::ProceedStopRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const {
	// запрос информации об остановке:
	StopResponse response = request_handler.GetStopInfo(request.AsMap().at("name").AsString());

	if (response.stop_exist == false) {
		responses.StartDict()
			.Key("error_message"s).Value("not found"s)
			.Key("request_id"s).Value(request.AsMap().at("id").AsInt())
			.EndDict();
	}
	else {
		responses.StartDict()
			.Key("buses"s).StartArray();

		for (const auto& bus : response.buses) {
			responses.Value(std::string(bus));
		}

		responses.EndArray()
			.Key("request_id"s).Value(request.AsMap().at("id").AsInt())
			.EndDict();
	}
}

void JsonReader::ProceedMapRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const {
	// запрос на рисование карты
	svg::Document doc = request_handler.RenderMap();

	// документ конвертировать в строку, с экранированием спецсимволов
	std::ostringstream strm;
	doc.Render(strm);

	std::string svg_document = strm.str();

	responses.StartDict()
		.Key("map"s).Value(svg_document)
		.Key("request_id"s).Value(request.AsMap().at("id").AsInt())
		.EndDict();
}

void JsonReader::ProceedRouteRequest(const RequestHandler& request_handler, json::Builder& responses, const json::Node& request) const {
	std::string_view from = request.AsMap().at("from").AsString();
	std::string_view to = request.AsMap().at("to").AsString();
	std::optional<RouteResponse> response = request_handler.GetRoute(from, to);

	if (!response) {
		responses.StartDict()
			.Key("request_id"s).Value(request.AsMap().at("id").AsInt())
			.Key("error_message"s).Value("not found"s)
			.EndDict();
	}
	else {
		responses.StartDict()
			.Key("request_id"s).Value(request.AsMap().at("id").AsInt())
			.Key("total_time"s).Value(response.value().total_time)
			.Key("items"s).StartArray();

		for (const auto& time_cut : response.value().time_cuts) {
			// в зависимости от того, Wait или Bus, по разному обрабатывать
			responses.StartDict();

			if (std::holds_alternative<Wait>(time_cut)) {
				const Wait wait = std::get<Wait>(time_cut);
				responses.Key("stop_name"s).Value(std::string(wait.stop_name));
				responses.Key("time"s).Value(wait.time);
				responses.Key("type"s).Value("Wait"s);
			}
			else if (std::holds_alternative<RidingBus>(time_cut)) {
				const RidingBus bus = std::get<RidingBus>(time_cut);
				responses.Key("bus"s).Value(std::string(bus.bus_name));
				responses.Key("span_count"s).Value(static_cast<int>(bus.span_count));
				responses.Key("time"s).Value(bus.time);
				responses.Key("type"s).Value("Bus"s);
			}

			responses.EndDict();
		}

		responses.EndArray().EndDict();
	}
}

void JsonReader::RequestAndPrint(const RequestHandler& request_handler, std::ostream& out) const {
	json::Builder responses = json::Builder{};
	responses.StartArray();

	for (const auto& request : stat_requests_) {
		if (request.AsMap().at("type").AsString() == "Bus") {
			ProceedBusRequest(request_handler, responses, request);
		}
		else if (request.AsMap().at("type").AsString() == "Stop") {
			ProceedStopRequest(request_handler, responses, request);
		}
		else if (request.AsMap().at("type").AsString() == "Map") {
			ProceedMapRequest(request_handler, responses, request);
		}
		else if (request.AsMap().at("type").AsString() == "Route") {
			ProceedRouteRequest(request_handler, responses, request);
		}
	}

	responses.EndArray();

	json::Document doc(responses.Build());
	Print(doc, out);
}

void JsonReader::Print(json::Document& doc, std::ostream& out) const {
	json::Print(doc, out);
}

void JsonReader::ParseStop(json::Dict dict) {
	/*
	{
		"type": "Stop",
		"name": "Ривьерский мост",
		"latitude": 43.587795,
		"longitude": 39.716901,
		"road_distances": {"Морской вокзал": 850}
	},
	*/

	std::string stopname = dict.at("name").AsString();
	double latitude = dict.at("latitude").AsDouble();
	double longitude = dict.at("longitude").AsDouble();

	stops_buffer_.push_back({ stopname, { latitude, longitude } });

	for (const auto& [name, dist] : dict.at("road_distances").AsMap()) {
		stops_distances_buffer_.insert({ { stopname, name }, dist.AsInt() });
	}
}

void JsonReader::ParseBus(json::Dict dict) {
	/*
	{
		"type": "Bus",
		"name": "114",
		"stops": ["Морской вокзал", "Ривьерский мост"],
		"is_roundtrip": false
	},
	*/

	std::string busname = dict.at("name").AsString();
	json::Array stops_middle_buffer = dict.at("stops").AsArray(); // Nodes
	bool is_round = dict.at("is_roundtrip").AsBool();

	if (is_round == false) {
		// проход туда и обратно
		std::vector<std::string> bidirectional_middle_buffer;
		for (const auto& stop : stops_middle_buffer) {
			bidirectional_middle_buffer.push_back(stop.AsString());
		}
		for (auto it = stops_middle_buffer.rbegin() + 1; it != stops_middle_buffer.rend(); ++it) {
			bidirectional_middle_buffer.push_back(it->AsString());
		}
		buses_buffer_.push_back({ std::move(busname), std::move(bidirectional_middle_buffer), is_round });
	}
	else {
		// просто перенести в буфер всех маршрутов.
		std::vector<std::string> straight_middle_buffer;
		for (const auto& stop : stops_middle_buffer) {
			straight_middle_buffer.push_back(stop.AsString());
		}
		buses_buffer_.push_back({ std::move(busname), std::move(straight_middle_buffer), is_round });
	}
}

std::size_t DistancesBufferHasher::operator()(const DistancesBufferKey& key) const {
	return std::hash<std::string>()(key.first) + 23 * std::hash<std::string>()(key.second);
}

bool operator==(const DistancesBufferKey& lhs, const DistancesBufferKey& rhs) {
	return lhs.first == rhs.first && lhs.second == rhs.second;
}

} // namespace json_reader