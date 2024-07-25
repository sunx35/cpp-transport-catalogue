#include "request_handler.h"

/*
 * ����� ����� ���� �� ���������� ��� ����������� �������� � ����, ����������� ������, ������� ��
 * �������� �� �������� �� � transport_catalogue, �� � json reader.
 *
 * ���� �� ������������� �������, ��� ����� ���� �� ��������� � ���� ����,
 * ������ �������� ��� ������.
 */

 // ���������� ���������� � �������� (������ Bus)
BusResponse RequestHandler::GetBusInfo(const std::string_view& bus_name) const {
	return db_.GetBusInfo(bus_name);
}

// ���������� ��������, ���������� �����
StopResponse RequestHandler::GetStopInfo(const std::string_view& stop_name) const {
	return db_.GetStopInfo(stop_name);
}

BusesTable RequestHandler::GetAllBuses() const {
	return db_.GetAllBuses();
}

svg::Document RequestHandler::RenderMap() const {
	return renderer_.PrintMap(GetAllBuses());
}