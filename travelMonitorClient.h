#ifndef TREVELMONITORCLIENT_H
#define TREVELMONITORCLIENT_H

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "DataStructures/monitorCountryPairList/monitorCountryPairList.h"
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/stringList/stringList.h"
#include "DataStructures/statsList/statsList.h"
#include "DataStructures/monitorList/monitorList.h"

using namespace std;

class travelMonitorClient {
public:
    travelMonitorClient(int m, int b, int s, string dir);
    travelMonitorClient();
    // ~travelMonitorClient();
    void start(int m, int b, int c, int s, string dir, int t);

    void findIP();
    void roundRobinCountriesandPutToList();
    void createServers();
    void createServer(int i);
    void openFifos();
    void openFifo(int i);
    void sendCredentials();
    void sendCredential(int i);
    void sendCountries();
    void sendCountry(string country, int i, bool first);
    void receiveBlooms();
    void receiveBlooms(int i);
    void sendDone();
    void startMenu();

    void travelRequest(string* arguments, int length);
    void travelStats(string* arguments, int length);
    void addVaccinationRecords(string* arguments, int length);
    void searchVaccinationStatus(string* arguments, int length);

    void suicide();
    void sendSIGUSR1(int monitor);
    void sendSIGINT(int monitor);
    void sendSIGKILL(int monitor);
    void sendSIGTERM(int monitor);
    void killAllMonitors();

    void sendStr(int monitor, string str);
    string receiveStr(int monitor);
    string receiveManyStr(int monitor, int* end);

    void addCountryToMonitor(string c, int m);
    void addMonitor(int pid, int id);
    void addFdToMonitor(int m, int r, int w);

    void addRequest(string c, string v, date dt, bool s);
    void addNewVirus(string virusName);
    void addNewCountry(string countryName);

    void printAllViruses();

    void printCountryToMonitor();

    monitorList* getMonitors() { return this->monitors; }
    monitorCountryPairList* getCountryToMonitor() { return this->countryToMonitor; }

private:
    struct hostent* ip;
    struct in_addr** ip_addr;
    char machineName[256];
    char externalAddress[256];

    string* command;
    int numMonitors;
    int socketBufferSize;
    int cyclicBufferSize;
    int sizeOfBloom;
    string input_dir;
    int numThreads;

    monitorList* monitors;
    monitorCountryPairList* countryToMonitor;

    stringList* viruses;
    statsList* requests;
    stringList* filePaths;
    stringList* countries;
    bloomFilterList* blooms;
};

#endif