/*
    This files contains the statsList class that is
    used to store the virus and countries names to avoid country duplication.

    Every time a country name or a virus name is needed a pointer to the node is made.
*/

#ifndef STATSLIST_H
#define STATSLIST_H

#include <string>
#include "../date/date.h"

using namespace std;

class statsList
{
public:
    statsList();
    statsList(string c, string v, date dt, bool s);
    ~statsList();

    void printNode(); // print only this node
    void print();     // print this and all the next nodes

    statsList* add(string c, string v, date dt, bool s);

    // GETTERS
    string getCountry() { return this->country; }
    string getVirusName() { return this->virusName; }
    date getDate() { return this->d; }
    bool getStat() { return this->stat; }
    statsList* getNext() { return this->next; }

    // SETTERS
    void setCountry(string s) { this->country = s; }
    void setVirusName(string s) { this->virusName = s; }
    void setDate(date s) { this->d = s; }
    void setStat(bool s) { this->stat = s; }
    void setNext(statsList* n) { this->next = n; }

private:
    string country;
    string virusName;
    date d;
    bool stat;
    statsList* next;
};

#endif