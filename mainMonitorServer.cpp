#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

#include "monitorServer.h"
#include "util.h"

extern monitorServer monitor;

int main(int argc, char* argv[])
{
    monitor.start(argv[0], argv[1]);
    monitor.receiveCredentials();
    monitor.receiveCountries();
    monitor.readFilesAndCreateStructures();
    monitor.printAllCountries();
    monitor.sendBlooms();
    while (1) {
        monitor.waitForCommands();
    }
    return 0;
}