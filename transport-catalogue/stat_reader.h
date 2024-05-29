#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <stdexcept>
#include <string_view>

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& catalogue, std::string_view line, std::ostream& out);