#include "map_renderer.h"

#include <cmath>
#include <map>
#include <sstream>
#include <string>
#include <variant>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {

bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}

void MapRenderer::SetWidth(double width) {
	if (width < -EPSILON || width > 100000) {
		throw std::invalid_argument("invalid width");
	}
	width_ = width;
}

void MapRenderer::SetHeight(double height) {
	if (height < -EPSILON || height > 100000) {
		throw std::invalid_argument("invalid height");
	}
	height_ = height;
}

void MapRenderer::SetPadding(double padding) {
	if (padding < -EPSILON || padding > std::min(width_, height_) / 2) {
		throw std::invalid_argument("invalid padding");
	}
	padding_ = padding;
}

void MapRenderer::SetStopRadius(double stop_radius) {
	if (stop_radius < -EPSILON || stop_radius > 100000) {
		throw std::invalid_argument("invalid stop_radius");
	}
	stop_radius_ = stop_radius;
}

void MapRenderer::SetLineWidth(double line_width) {
	if (line_width < -EPSILON || line_width > 100000) {
		throw std::invalid_argument("invalid line_width");
	}
	line_width_ = line_width;
}

void MapRenderer::SetBusLabelFontSize(int bus_label_font_size) {
	if (bus_label_font_size < -EPSILON || bus_label_font_size > 100000) {
		throw std::invalid_argument("invalid bus_label_font_size");
	}
	bus_label_font_size_ = bus_label_font_size;
}

void MapRenderer::SetBusLabelOffset(svg::Point bus_label_offset) {
	if (bus_label_offset.x < -100000 || bus_label_offset.x > 100000 ||
		bus_label_offset.y < -100000 || bus_label_offset.y > 100000) {
		throw std::invalid_argument("invalid bus_label_offset");
	}
	bus_label_offset_ = bus_label_offset;
}

void MapRenderer::SetStopLabelFontSize(int stop_label_font_size) {
	if (stop_label_font_size < -EPSILON || stop_label_font_size > 100000) {
		throw std::invalid_argument("invalid stop_label_font_size");
	}
	stop_label_font_size_ = stop_label_font_size;
}

void MapRenderer::SetStopLabelOffset(svg::Point stop_label_offset) {
	if (stop_label_offset.x < -100000 || stop_label_offset.x > 100000 ||
		stop_label_offset.y < -100000 || stop_label_offset.y > 100000) {
		throw std::invalid_argument("invalid bus_label_offset");
	}
	stop_label_offset_ = stop_label_offset;
}

void MapRenderer::SetUnderlayerColor(svg::Color underlayer_color) {
	underlayer_color_ = underlayer_color;
}

void MapRenderer::SetUnderlayerWidth(double underlayer_width) {
	if (underlayer_width < -EPSILON || underlayer_width > 100000) {
		throw std::invalid_argument("invalid underlayer_width");
	}
	underlayer_width_ = underlayer_width;
}

void MapRenderer::SetColorPalette(std::vector<svg::Color> color_palette) {
	if (color_palette.empty()) {
		throw std::invalid_argument("Empty array for Color");
	}
	color_palette_ = color_palette;
}

svg::Document MapRenderer::PrintMap(BusesTable buses_table) const {
	std::map<std::string_view, Bus*> sorted_buses_table;
	for (const auto& bus : buses_table) {
		sorted_buses_table.insert(bus);
	}

	std::vector<geo::Coordinates> all_points;
	for (const auto& [_, bus] : sorted_buses_table) {
		for (const auto& stop : bus->stops) {
			all_points.push_back(stop->coordinates);
		}
	}

	SphereProjector projector{ all_points.begin(), all_points.end(), width_, height_, padding_ };

	svg::Document doc;

	// сначала все линии, потом все тексты маршрутов, потом все кружочки и т.д.

	{
		auto color_it = color_palette_.begin();
		for (const auto& [_, bus] : sorted_buses_table) {
			if (bus->stops.empty()) {
				continue;
			}

			// Линия маршрута

			svg::Polyline line;

			line.SetFillColor("none").SetStrokeColor(*color_it).SetStrokeWidth(line_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			for (const auto& stop : bus->stops) {
				line.AddPoint(projector(stop->coordinates));
			}
			doc.Add(line);

			if (std::next(color_it) == color_palette_.end()) {
				color_it = color_palette_.begin();
			}
			else {
				++color_it;
			}
		}
	}

	{
		auto color_it = color_palette_.begin();
		for (const auto& [_, bus] : sorted_buses_table) {
			if (bus->stops.empty()) {
				continue;
			}

			// Название маршрута

			svg::Text bus_label;
			svg::Text bus_label_underlayer;

			auto first_stop_ptr = *(bus->stops.begin());

			bus_label.SetPosition(projector(first_stop_ptr->coordinates)).SetOffset(bus_label_offset_)
				.SetFontSize(bus_label_font_size_).SetFontFamily("Verdana").SetFontWeight("bold")
				.SetData(bus->name);
			bus_label_underlayer = bus_label;
			bus_label_underlayer.SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_)
				.SetStrokeWidth(underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			bus_label.SetFillColor(*color_it);

			doc.Add(bus_label_underlayer);
			doc.Add(bus_label);

			// вторая конечная остановка находится посередине массива
			auto second_end_stop_ptr = bus->stops.at(bus->stops.size() / 2);
			auto second_end_stop_point = projector(second_end_stop_ptr->coordinates);

			// если это не круговой маршрут
			if (!bus->is_round && first_stop_ptr != second_end_stop_ptr) {
				svg::Text bus_label2 = bus_label;
				svg::Text bus_label2_underlayer = bus_label_underlayer; // вторая конечная остановка

				bus_label2.SetPosition(second_end_stop_point);
				bus_label2_underlayer.SetPosition(second_end_stop_point);

				doc.Add(bus_label2_underlayer);
				doc.Add(bus_label2);
			}

			if (std::next(color_it) == color_palette_.end()) {
				color_it = color_palette_.begin();
			}
			else {
				++color_it;
			}
		}
	}

	{
		// создаем уникальные и упорядоченные остановки

		std::map<std::string_view, Stop*> sorted_stops;

		for (const auto& [_, bus] : sorted_buses_table) {
			if (bus->stops.empty()) {
				continue;
			}

			for (const auto& stop : bus->stops) {
				sorted_stops.insert({ stop->name, stop });
			}
		}

		// Кружочки остановок

		for (const auto& [_, stop] : sorted_stops) {
			svg::Circle circle;
			circle.SetCenter(projector(stop->coordinates)).SetRadius(stop_radius_).SetFillColor("white");
			doc.Add(circle);
		}

		// Текст остановок

		for (const auto& [_, stop] : sorted_stops) {
			svg::Text stop_label;
			svg::Text stop_label_underlayer;
			stop_label.SetPosition(projector(stop->coordinates)).SetOffset(stop_label_offset_)
				.SetFontSize(stop_label_font_size_).SetFontFamily("Verdana")
				.SetData(stop->name);
			stop_label_underlayer = stop_label;
			stop_label_underlayer.SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_)
				.SetStrokeWidth(underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			stop_label.SetFillColor("black");
			doc.Add(stop_label_underlayer);
			doc.Add(stop_label);
		}
	}

	return doc;
}

} // namespace renderer