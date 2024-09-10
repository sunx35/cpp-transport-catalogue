#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"
//#include "tests.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace transport_catalogue;
using namespace json_reader;
using namespace renderer;
using namespace router;

int main() {
    //RunTests();

    JsonReader reader;
    reader.ReadInput(cin);
    const TransportCatalogue catalogue = reader.CreateDatabase();
    const MapRenderer renderer = reader.CreateMapRenderer();
    const TransportRouter router = reader.CreateTransportRouter(catalogue);

    const RequestHandler request_handler(catalogue, renderer, router);

    reader.RequestAndPrint(request_handler, cout);
}