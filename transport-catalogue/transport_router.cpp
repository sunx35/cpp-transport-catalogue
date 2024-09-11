#include "transport_router.h"

#include <vector>

namespace router {

TransportRouter::TransportRouter(RoutingSettings settings, const transport_catalogue::TransportCatalogue& catalogue)
    : settings_(settings)
    , catalogue_(catalogue) {
        InitializeStops();
        InitializeGraph();
}

graph::VertexId TransportRouter::FindVertexIdByStopName(std::string_view stop_name) const {
    return stops_ids_table_.at(stop_name);
}

Timecut TransportRouter::FindTimecutByEdgeId(graph::EdgeId edge_id) const {
    return time_cuts_.at(edge_id);
}

std::optional<RouteResponse> TransportRouter::GetRoute(std::string_view from, std::string_view to) const {
    if (router_ == nullptr) {
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }
    
    auto route_info = router_->BuildRoute(FindVertexIdByStopName(from), FindVertexIdByStopName(to));
    if (!route_info) {
        return std::nullopt;
    }
    RouteResponse response;
    response.total_time = route_info.value().weight;

    for (const auto edge_id : route_info.value().edges) {
        response.time_cuts.push_back(FindTimecutByEdgeId(edge_id));
    }

    return response;
}

void TransportRouter::InitializeStops() {
    // ������� ��� ����������� VertexId � �������� ���������
    graph::VertexId vertexId = 0;
    for (const auto& [stop_name, _] : catalogue_.GetAllStops()) {
        stops_ids_table_.insert({ stop_name, vertexId });
        // ������ ��������� ����� �������� �� ���� ���������.
        // �� ������ ������� ����� ������������, �� �� ����� ������� ��� � ����������,
        // � ����� ������ ��������� +1 ���, ��� ���� ������� �����.
        vertexId += 2;
    }
}

void TransportRouter::InitializeGraph() {
    graph_ = graph::DirectedWeightedGraph<double>{ catalogue_.GetAllStops().size() * 2 };

    // ������� ��������� � ���� ��� ���� ��� �������� �� ����������
    for (const auto& [stop_name, vertexId] : stops_ids_table_) {
        auto edge_id = graph_.AddEdge({ vertexId, vertexId + 1, settings_.bus_wait_time });
        time_cuts_[edge_id] = Wait{ settings_.bus_wait_time, stop_name };
    }

    // ����� ��� �������� ������� ���������� ����� �����������
    // ��������: A -> B, A -> C, A -> D; B -> C, B -> D � �.�.
    // (���������� � D -> D ������� �� �����, �������
    // ��� ������� ������� ������ � ������������� �� ���������)
    BusesTable buses_table = catalogue_.GetAllBuses();
    for (const auto& [name, bus_ptr] : buses_table) {
        CreateEdgesBetweenStops(name, bus_ptr);
    }
}

void TransportRouter::CreateEdgesBetweenStops(std::string_view bus_name, const Bus* const bus_ptr) {
    auto stops = bus_ptr->stops;
    for (size_t i = 0; i < stops.size() - 1; ++i) {
        // ���� ������� �� ��������, ����� �������� ������ �� �������� ���������
        if (!bus_ptr->is_round && i >= stops.size() / 2) {
            break;
        }
        for (size_t j = i + 1; j < stops.size(); ++j) {
            // ���� ������� �� ��������, ����� �������� ������ �� �������� ���������
            if (!bus_ptr->is_round && j > stops.size() / 2) {
                break;
            }
            // ������� ��������� ������� (�� ���������� ���������)
            // ���� � �������
            double stops_distance = 0;
            double stops_distance_inverse = 0;
            for (size_t k = i + 1; k <= j; ++k) {
                stops_distance += catalogue_.GetDistance(stops[k - 1], stops[k]);
                stops_distance_inverse += catalogue_.GetDistance(stops[k], stops[k - 1]);
            }
            auto edge_id = graph_.AddEdge({
                FindVertexIdByStopName(stops[i]->name) + 1, // �� ����� +1 ��� ��������� ��������
                FindVertexIdByStopName(stops[j]->name),
                stops_distance / METERS_IN_KILOMETERS / settings_.bus_velocity * MINUTES_IN_HOUR });
            time_cuts_[edge_id] = RidingBus{
                /*time*/ stops_distance / METERS_IN_KILOMETERS / settings_.bus_velocity * MINUTES_IN_HOUR,
                /*bus_name*/ bus_name,
                /*span_count*/ j - i };

            // ���� ��� �� �������� �������, ����� ����� �������� ���������
            if (!bus_ptr->is_round) {
                auto edge_id = graph_.AddEdge({
                    FindVertexIdByStopName(stops[j]->name) + 1,
                    FindVertexIdByStopName(stops[i]->name),
                    stops_distance_inverse / METERS_IN_KILOMETERS / settings_.bus_velocity * MINUTES_IN_HOUR });
                time_cuts_[edge_id] = RidingBus{
                    /*time*/ stops_distance_inverse / METERS_IN_KILOMETERS / settings_.bus_velocity * MINUTES_IN_HOUR,
                    /*bus_name*/ bus_name,
                    /*span_count*/ j - i };
            }
        }
    }
}

} // namespace router