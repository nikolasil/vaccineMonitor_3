#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

#include "monitorServer.h"
#include "util.h"

extern monitorServer monitor;

int main(int argc, char* argv[])
{
    int numPaths = argc - 10;
    char* paths[numPaths];
    for (int i = 0;i < numPaths;i++)
        paths[i] = argv[10 + i];

    monitor.start(stoi(argv[1]), stoi(argv[3]), stoi(argv[5]), stoi(argv[7]), stoi(argv[9]), paths, numPaths);
    monitor.initializeServer();
    monitor.receiveId();
    monitor.openThreads();
    monitor.sendBlooms();
    while (1) {
        monitor.waitForCommands();
    }
    return 0;
}