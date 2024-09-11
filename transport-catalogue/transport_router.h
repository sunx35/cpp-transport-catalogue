#pragma once

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace router {

static const int MINUTES_IN_HOUR = 60;
static const int METERS_IN_KILOMETERS = 1000;

class TransportRouter {
public:
    TransportRouter(RoutingSettings settings, const transport_catalogue::TransportCatalogue& catalogue);
    std::optional<RouteResponse> GetRoute(std::string_view from, std::string_view to) const;

private:
    graph::VertexId FindVertexIdByStopName(std::string_view stop_name) const;
    Timecut FindTimecutByEdgeId(graph::EdgeId edge_id) const;

    void InitializeStops();
    void InitializeGraph();

    void CreateEdgesBetweenStops(std::string_view bus_name, const Bus* const bus_ptr);

    RoutingSettings settings_;
    const transport_catalogue::TransportCatalogue& catalogue_;

    std::unordered_map<std::string_view, graph::VertexId> stops_ids_table_;
    std::unordered_map<graph::EdgeId, Timecut> time_cuts_;
    graph::DirectedWeightedGraph<double> graph_;

    mutable std::unique_ptr<graph::Router<double>> router_ = nullptr;
};

} // namespace router