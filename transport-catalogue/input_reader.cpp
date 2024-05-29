#include "input_reader.h"

namespace input_reader {

void InputReader::ParseLine(std::string_view line) {
	size_t command_first_char = line.find_first_not_of(' ');

	if (line[command_first_char] == 'S') {
		// Stop X: latitude, longitude

		size_t stopname_begin_pos = line.find_first_not_of(' ', command_first_char + 4);
		size_t stopname_end_pos = line.find_first_of(':', stopname_begin_pos);
		size_t stopname_size = stopname_end_pos - stopname_begin_pos;
		std::string_view stopname = line.substr(stopname_begin_pos, stopname_size);
		if (stopname.back() == ' ') {
			stopname = stopname.substr(0, stopname.find_last_not_of(' ') + 1);
		}

		size_t latitude_begin_pos = line.find_first_not_of(' ', stopname_end_pos + 1);
		size_t latitude_end_pos = line.find_first_of(',');
		size_t latitude_size = latitude_end_pos - latitude_begin_pos;
		size_t longitude_begin_pos = line.find_first_not_of(' ', latitude_end_pos + 1);
		size_t longitude_end_pos = line.size();
		size_t longitude_size = longitude_end_pos - longitude_begin_pos;
		std::string_view latitude = line.substr(latitude_begin_pos, latitude_size);
		if (latitude.back() == ' ') {
			latitude = latitude.substr(0, latitude.find_last_not_of(' ') + 1);
		}
		std::string_view longitude = line.substr(longitude_begin_pos, longitude_size);
		if (longitude.back() == ' ') {
			longitude = longitude.substr(0, longitude.find_last_not_of(' ') + 1);
		}
		double latitude_d = std::stod(latitude.data());
		double longitude_d = std::stod(longitude.data());

		stops_buffer_.push_back({ std::move(std::string(stopname)), { latitude_d, longitude_d } }); // нужно ли мувить структуру?
	}
	else {
		// Bus X: stop1 - stop2 - ... stopN
		// Bus X: stop1 > stop2 > ... > stopN > stop1

		size_t busname_begin_pos = line.find_first_not_of(' ', command_first_char + 3);
		size_t busname_end_pos = line.find_first_of(':', busname_begin_pos);
		size_t busname_size = busname_end_pos - busname_begin_pos;
		std::string_view busname = line.substr(busname_begin_pos, busname_size);
		if (busname.back() == ' ') {
			busname = busname.substr(0, busname.find_last_not_of(' ') + 1);
		}

		// Поиск и обработка остановок маршрута

		auto stops_middle_buffer = ParseStops(line, busname_end_pos);

		if (line.find('-') != line.npos) {
			// проход туда и обратно
			std::vector<std::string> bidirectional_middle_buffer;
			for (const auto& stop : stops_middle_buffer) {
				bidirectional_middle_buffer.push_back(stop);
			}
			for (auto it = stops_middle_buffer.rbegin() + 1; it != stops_middle_buffer.rend(); ++it) {
				bidirectional_middle_buffer.push_back(*it);
			}
			buses_buffer_.push_back({ std::move(std::string(busname)), bidirectional_middle_buffer });
		}
		else {
			// просто перенести в буфер всех маршрутов.
			buses_buffer_.push_back({ std::move(std::string(busname)), stops_middle_buffer }); // нужно ли мувить структуру?
		}
	}
}

void InputReader::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) {
	// добавить все остановки
	for (transport_catalogue::Stop& stop : stops_buffer_) {
		catalogue.AddStop(std::move(stop));
	}

	// добавить все автобусы
	for (auto& bus_buffer : buses_buffer_) {
		transport_catalogue::Bus bus;
		bus.name = bus_buffer.bus_name;
		for (auto& stopname : bus_buffer.stops_names) {
			bus.stops.push_back(catalogue.FindStopByName(stopname));
		}
		catalogue.AddBus(std::move(bus));
	}
}

std::vector<std::string> InputReader::ParseStops(const std::string_view& line, size_t busname_end_pos) {
	std::vector<std::string> stops_middle_buffer;

	size_t stop1_begin_pos = line.find_first_not_of(' ', busname_end_pos + 1);
	size_t stop1_end_pos = line.find_first_of("->", stop1_begin_pos);
	size_t stop1_size = stop1_end_pos - stop1_begin_pos;
	std::string stop1 = std::string(line.substr(stop1_begin_pos, stop1_size));
	if (stop1.back() == ' ') {
		stop1 = stop1.substr(0, stop1.find_last_not_of(' ') + 1);
	}

	stops_middle_buffer.push_back(std::move(stop1));

	size_t stop2_begin_pos = line.find_first_not_of(' ', stop1_end_pos + 1);
	size_t stop2_end_pos = line.find_first_of("->", stop2_begin_pos); // найдет либо '-', либо npos
	size_t stop2_size;
	if (stop2_end_pos == line.npos) {
		stop2_size = line.size() - stop2_begin_pos;
	}
	else {
		stop2_size = stop2_end_pos - stop2_begin_pos;
	}
	std::string stop2 = std::string(line.substr(stop2_begin_pos, stop2_size));
	if (stop2.back() == ' ') {
		stop2 = stop2.substr(0, stop2.find_last_not_of(' ') + 1);
	}

	stops_middle_buffer.push_back(std::move(stop2));

	// если это не конец строки, мы ищем и добавляем еще одну остановку в middle_buffer
	if (stop2_end_pos != line.npos) {
		FindAndAddAnotherStop(stops_middle_buffer, line, stop2_end_pos);
	}

	return stops_middle_buffer;
}

void InputReader::FindAndAddAnotherStop(std::vector<std::string>& stops_middle_buffer, const std::string_view& line, size_t prev_stop_end_pos) {
	size_t stopN_begin_pos = line.find_first_not_of(' ', prev_stop_end_pos + 1);
	size_t stopN_end_pos = line.find_first_of("->", stopN_begin_pos); // найдет либо '-', либо npos
	size_t stopN_size;
	if (stopN_end_pos == line.npos) {
		stopN_size = line.size() - stopN_begin_pos;
	}
	else {
		stopN_size = stopN_end_pos - stopN_begin_pos;
	}
	std::string stopN = std::string(line.substr(stopN_begin_pos, stopN_size));
	if (stopN.back() == ' ') {
		stopN = stopN.substr(0, stopN.find_last_not_of(' ') + 1);
	}

	stops_middle_buffer.push_back(std::move(stopN));

	if (stopN_end_pos != line.npos) {
		FindAndAddAnotherStop(stops_middle_buffer, line, stopN_end_pos);
	}
}

} // namespace input_reader