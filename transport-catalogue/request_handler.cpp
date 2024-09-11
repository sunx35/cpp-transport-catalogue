/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

#include "request_handler.h"

BusResponse RequestHandler::GetBusInfo(std::string_view bus_name) const {
    return db_.GetBusInfo(bus_name);
}

StopResponse RequestHandler::GetStopInfo(std::string_view stop_name) const {
    return db_.GetStopInfo(stop_name);
}

std::optional<RouteResponse> RequestHandler::GetRoute(std::string_view from, std::string_view to) const {
    return router_.GetRoute(from, to);
}

BusesTable RequestHandler::GetAllBuses() const {
    return db_.GetAllBuses();
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.PrintMap(GetAllBuses());
}