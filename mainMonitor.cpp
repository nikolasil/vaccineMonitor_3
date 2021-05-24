#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

#include "monitor.h"
#include "util.h"

extern Monitor monitor;

int main(int argc, char* argv[])
{
    string pipe0 = argv[0];
    string pipe1 = argv[1];
    // cout << argc << endl;
    if (argc != 2)
        cout << "Arguments must be 2" << endl;
    // check if pipes exists !
    // Monitor monitor(pipe0, pipe1);
    monitor.start(pipe0, pipe1);
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