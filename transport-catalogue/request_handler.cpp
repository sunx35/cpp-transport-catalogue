#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // Возвращает информацию о маршруте (запрос Bus)
BusResponse RequestHandler::GetBusInfo(const std::string_view& bus_name) const {
	return db_.GetBusInfo(bus_name);
}

// Возвращает маршруты, проходящие через
StopResponse RequestHandler::GetStopInfo(const std::string_view& stop_name) const {
	return db_.GetStopInfo(stop_name);
}

BusesTable RequestHandler::GetAllBuses() const {
	return db_.GetAllBuses();
}

svg::Document RequestHandler::RenderMap() const {
	return renderer_.PrintMap(GetAllBuses());
}