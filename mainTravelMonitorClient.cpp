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

    // travelMonitorClient mainMonitor = travelMonitorClient(numMonitors, bufferSize, sizeOfBloom, input_dir);
    mainMonitor.start(numMonitors, socketBufferSize, cyclicBufferSize, sizeOfBloom, input_dir, numThreads);
    mainMonitor.findIP();
    mainMonitor.createServers();
    // mainMonitor.openFifos();
    // mainMonitor.sendCredentials();
    // mainMonitor.sendCountries();
    // mainMonitor.receiveBlooms();
    // mainMonitor.printAllViruses();
    // while (1) {
    //     mainMonitor.startMenu();
    // }
    // return 0;
}