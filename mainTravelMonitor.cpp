#include <iostream>
#include <string>
#include <unistd.h>

#include "travelMonitor.h"
#include "util.h"

using namespace std;

extern travelMonitor mainMonitor;

int main(int argc, char* argv[])
{
    int numMonitors;
    int socketBufferSize;
    int cyclicBufferSize;
    int sizeOfBloom;
    string input_dir;
    int numThreads;
    if (!checkArguments(argc, argv, numMonitors, socketBufferSize, cyclicBufferSize, sizeOfBloom, input_dir, numThreads))
        return 1;

    // travelMonitor mainMonitor = travelMonitor(numMonitors, bufferSize, sizeOfBloom, input_dir);
    mainMonitor.start(numMonitors, socketBufferSize, cyclicBufferSize, sizeOfBloom, input_dir, numThreads);
    mainMonitor.createFIFOs();
    mainMonitor.createMonitors();
    mainMonitor.openFifos();
    mainMonitor.sendCredentials();
    mainMonitor.sendCountries();
    mainMonitor.receiveBlooms();
    mainMonitor.printAllViruses();
    while (1) {
        mainMonitor.startMenu();
    }
    return 0;
}