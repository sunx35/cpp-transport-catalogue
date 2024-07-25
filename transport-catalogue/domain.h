#pragma once

#include "geo.h"

#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

/*
 * � ���� ����� �� ������ ���������� ������/���������, ������� �������� ������ ���������� ������� (domain)
 * ������ ���������� � �� ������� �� ������������� �����������. �������� ���������� �������� � ���������.
 *
 * �� ����� ���� �� ���������� � � transport_catalogue.h, ������ ��������� �� � ���������
 * ������������ ���� ����� ��������� ��������, ����� ���� ����� �� ������������ ����� ���������:
 * ������������ ����� (map_renderer) ����� ����� ������� ����������� �� ������������� �����������.
 *
 * ���� ��������� ������ ���������� �� ��������� ��� �������, ������ �������� ���� ���� ������.
 *
 */

 // �� ��������� �� ���������� namespace, ��� ��� domain, ��� �������� �������, ������� �����������
 // �� ������ ������ ���������.

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