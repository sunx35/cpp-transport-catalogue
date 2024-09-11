/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
 // с другими подсистемами приложения.
 // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>
 
class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer, 
        const router::TransportRouter& router)
        : db_(db)
        , renderer_(renderer)
        , router_(router) {
    }

    BusResponse GetBusInfo(std::string_view bus_name) const;
    StopResponse GetStopInfo(std::string_view stop_name) const;
    std::optional<RouteResponse> GetRoute(std::string_view from, std::string_view to) const;
    BusesTable GetAllBuses() const;
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const router::TransportRouter& router_;
};