#ifndef MONITORSERVER_H
#define MONITORSERVER_H

#include <iostream>
#include <string>
#include <signal.h>

#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/binaryAvlTree/tree.h"
#include "DataStructures/skipList/skipList.h"
#include "DataStructures/stringList/stringList.h"

using namespace std;

class monitorServer {
public:
    monitorServer();
    ~monitorServer();
    void start(int p, int t, int sb, int cb, int bloom, char** paths, int numPaths);
    void openPathsByThreads();

    void sendBlooms();
    void receiveDone();

    void suicide();
    void travelRequest(string* arguments, int length);
    void searchVaccinationStatus(string* arguments, int length);

    void addFromFile(string filePath);
    void addRecord(int length, string* words, string line);
    // void signal_handler_2(int signo);
    void addNewVirus(string virusName);
    void addNewCountry(string countryName);
    int addNewFile(string file);

    int checkSyntaxRecord(string errorMessage, int length, string* words, string input);

    void sendStr(string str);
    string receiveStr();
    string receiveManyStr(int* end);

    void printAllCountries();
    void printAllViruses();

    void makeLogFile();

    void waitForCommands();
private:
    int id;
    int port;
    int numThreads;
    string* command;
    int t;
    int f;
    string generalDirectory;

    char** argPaths;
    int argNumPaths;

    int socketBufferSize;
    int cyclicBufferSize;
    int bloomSize;

    treeNode* tree;
    bloomFilterList* blooms;
    skipList_List* skipLists;
    stringList* viruses;
    stringList* countries;
    stringList* filesReaded;
};

#endif