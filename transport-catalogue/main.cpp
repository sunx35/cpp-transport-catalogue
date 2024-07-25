#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
//#include "tests.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace transport_catalogue;
using namespace json_reader;
using namespace renderer;

int main() {
    //RunTests();

    TransportCatalogue catalogue;

    MapRenderer renderer;

    JsonReader reader;
    reader.ParseLine(cin);
    reader.FillDatabase(catalogue);
    reader.SetupMapRenderer(renderer);

    RequestHandler request_handler(catalogue, renderer);

    //reader.RequestAndPrint(request_handler, cout);

    // ������� ����������� ������� �� reader'�.
    // ����� ��� ������� ���������� � request_handler.
    // ������ ����� ����� ���������� � JSON � ������� reader'�.

    auto stat_requests = reader.GetRequests(); // ����� ���� �� ��������� ��������� ���������.
    // � ��� ���� ������ � json_reader (json::Array, json::Dict � �.�.)

    vector<json::Node> responses; // json::Array

    for (const auto& request : stat_requests) {
        // ������ ���������� �� ��������:
        if (request.type == "Bus") {
            // ��������� ���������� �� ��������
            BusResponse response = request_handler.GetBusInfo(request.name);

            if (response.bus_exist == false) {
                json::Dict response_dict{
                    { "error_message", "not found"s },
                    { "request_id", json::Node(request.id) }
                };

                responses.push_back(response_dict);
            }
            else {
                // ����� �������� � json ��������
                json::Dict response_dict{
                    { "curvature", json::Node(response.curvature) },
                    { "request_id", json::Node(request.id) }, // �� ������
                    { "route_length", json::Node(response.route_length) },
                    { "stop_count", json::Node(static_cast<int>(response.stops_count)) },
                    { "unique_stop_count", json::Node(static_cast<int>(response.unique_stops_count)) }
                };

                responses.push_back(response_dict);
            }

            /* ��������������� �������, ��� ����� ���� �� �������� ��� � ������ �����
             *
            responses.emplace_back(json::Node{ json::Dict{
                { "curvature", json::Node(response.curvature) },
                { "request_id", json::Node(request.id) }, // �� ������
                { "route_length", json::Node(response.route_length) },
                { "stop_count", json::Node(static_cast<int>(response.stops_count)) },
                { "unique_stop_count", json::Node(static_cast<int>(response.unique_stops_count)) }
            } });
            */
        }
        else if (request.type == "Stop") {
            // ������ ���������� �� ���������:
            StopResponse response = request_handler.GetStopInfo(request.name);

            if (response.stop_exist == false) {
                json::Dict response_dict{
                    { "error_message", "not found"s },
                    { "request_id", json::Node(request.id) }
                };

                responses.push_back(response_dict);
            }
            else {
                std::vector<json::Node> buses;
                for (const auto& bus : response.buses) {
                    buses.push_back(json::Node(std::string(bus)));
                }

                json::Dict response_dict{
                    { "buses", json::Node(buses) },
                    { "request_id", request.id }
                };

                responses.push_back(response_dict);
            }
        }
        else if (request.type == "Map") {
            // ������ �� ��������� �����
            svg::Document doc = request_handler.RenderMap();

            // �������� �������������� � ������, � �������������� ������������
            std::ostringstream strm;
            doc.Render(strm);

            std::string svg_document = strm.str();

            json::Dict response_dict{
                { "map", svg_document },
                { "request_id", request.id }
            };

            responses.push_back(response_dict);
        }

    }

    json::Document doc(responses);
    reader.Print(doc, cout);
}