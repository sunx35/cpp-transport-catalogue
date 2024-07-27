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

    JsonReader reader;
    reader.ReadInput(cin);
    TransportCatalogue catalogue = reader.CreateDatabase();
    MapRenderer renderer = reader.CreateMapRenderer();

    RequestHandler request_handler(catalogue, renderer);

    reader.RequestAndPrint(request_handler, cout);
}