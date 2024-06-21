#include "transport_catalogue.h"
#include "input_reader.h"
//#include "tests.h"
#include "stat_reader.h"

#include <iostream>

using namespace std;
using namespace transport_catalogue;
using namespace input_reader;

int main() {
    //RunTests();

    TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        ParseAndPrintStat(catalogue, line, cout);
    }
}