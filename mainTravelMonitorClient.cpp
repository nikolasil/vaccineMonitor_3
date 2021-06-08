#include <iostream>
#include <string>
#include <unistd.h>

#include "travelMonitorClient.h"
#include "util.h"

using namespace std;

extern travelMonitorClient mainMonitor;

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

    mainMonitor.start(numMonitors, socketBufferSize, cyclicBufferSize, sizeOfBloom, input_dir, numThreads);
    mainMonitor.roundRobinCountriesandPutToList();
    mainMonitor.createServers();
    mainMonitor.openSockets();
    mainMonitor.sendIds();
    mainMonitor.receiveBlooms();
    while (1) {
        mainMonitor.startMenu();
    }
    // return 0;
}