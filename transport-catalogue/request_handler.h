#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>

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

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer,
        const router::TransportRouter& router)
        : db_(db)
        , renderer_(renderer)
        , router_(router) {
    }

    // Возвращает информацию о маршруте (запрос Bus)
    BusResponse GetBusInfo(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    StopResponse GetStopInfo(const std::string_view& stop_name) const; // maybe just string_view ?

    std::optional<RouteResponse> GetRoute(std::string_view from, std::string_view to) const;

    BusesTable GetAllBuses() const; // возможно пригодится какой-то кастомный контейнер для этих целей

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const router::TransportRouter& router_;
};