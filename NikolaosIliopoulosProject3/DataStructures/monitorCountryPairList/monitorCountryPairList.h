/*
    This files contains the monitorCountryPairList class that is
    used to store the virus and countries names to avoid country duplication.

    Every time a country name or a virus name is needed a pointer to the node is made.
*/

#ifndef monitorCountryPairList_H
#define monitorCountryPairList_H

#include <iostream>
#include <string>

using namespace std;

class monitorCountryPairList
{
public:
    monitorCountryPairList();
    monitorCountryPairList(string d, int m);
    ~monitorCountryPairList();

    void print();     // print this and all the next nodes

    monitorCountryPairList* add(string d, int m);
    monitorCountryPairList* remove(string d);
    monitorCountryPairList* search(string d);

    // GETTERS
    string getCountry() { return this->country; }
    monitorCountryPairList* getNext() { return this->next; }
    int getMonitor() { return this->monitor; }

    // SETTERS
    void setCountry(string s) { this->country = s; }
    void setNext(monitorCountryPairList* n) { this->next = n; }
    void setMonitor(int m) { this->monitor = m; }

private:
    string country;
    int monitor;
    monitorCountryPairList* next;
};

#endif