#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <optional>

/*
 * ����� ����� ���� �� ���������� ��� ����������� �������� � ����, ����������� ������, ������� ��
 * �������� �� �������� �� � transport_catalogue, �� � json reader.
 *
 * � �������� ��������� ��� ���� ���������� ��������� �� ���� ������ ����������� ��������.
 * �� ������ ����������� ��������� �������� ��������, ������� ������� ���.
 *
 * ���� �� ������������� �������, ��� ����� ���� �� ��������� � ���� ����,
 * ������ �������� ��� ������.
 */

 // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
 // � ������� ������������ ����������.
 // ��. ������� �������������� �����: https://ru.wikipedia.org/wiki/�����_(������_��������������)

class RequestHandler {
public:
    // MapRenderer ����������� � ��������� ����� ��������� �������
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer)
        : db_(db)
        , renderer_(renderer) {
    }

    // ���������� ���������� � �������� (������ Bus)
    BusResponse GetBusInfo(const std::string_view& bus_name) const;

    // ���������� ��������, ���������� �����
    StopResponse GetStopInfo(const std::string_view& stop_name) const;

    BusesTable GetAllBuses() const; // �������� ���������� �����-�� ��������� ��������� ��� ���� �����

    // ���� ����� ����� ����� � ��������� ����� ��������� �������
    svg::Document RenderMap() const;

private:
    // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};