#ifndef VACCINEMONITOR_H
#define VACCINEMONITOR_H

#include <iostream>
#include <string>
#include <signal.h>

#include "DataStructures/monitorCountryPairList/monitorCountryPairList.h"
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/stringList/stringList.h"
#include "DataStructures/statsList/statsList.h"
#include "DataStructures/monitorList/monitorList.h"

using namespace std;

class travelMonitor {
public:
    travelMonitor(int m, int b, int s, string dir);
    travelMonitor();
    // ~travelMonitor();
    void start(int m, int b, int s, string dir);

    void createFIFOs();
    void createMonitors();
    void createMonitor(int i);
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
    struct sigaction handlerSIGINT_SIGQUIT;
    struct sigaction handlerSIGCHLD;
    string* command;
    int numMonitors;
    int bufferSize;
    int sizeOfBloom;
    string input_dir;

    monitorList* monitors;
    monitorCountryPairList* countryToMonitor;

    stringList* viruses;
    statsList* requests;
    stringList* countries;
    bloomFilterList* blooms;
};

#endif