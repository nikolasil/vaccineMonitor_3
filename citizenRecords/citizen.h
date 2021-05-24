/*
    This file contains the citizenRecord class together with the listStatus class.

    Each citizen has his own listStatus that holds vaccination informations.
*/

#ifndef CITIZEN_H
#define CITIZEN_H

#include <string>

#include "../DataStructures/date/date.h"
#include "../DataStructures/stringList/stringList.h"

using namespace std;

class listStatus
    /* this class is a linked list that holds the
    the viruses statuses that a citizen can have */
{
public:
    // 2 different constructors where the date is string or class date
    listStatus(stringList* virus, char s, string d);
    listStatus(stringList* virus, char s, date d);
    ~listStatus();

    void print();

    void addStatus(stringList* virus, char s, date d); // add in the end of the list a new virus status 
    listStatus* getNode(stringList* virus);
    listStatus* getNode(string virus);

    char getVirusStatus(stringList* virus);     //   given the virus name return the status of that virus.
    char getVirusStatus(string virus);          //   '\0' if that virus is not in that citizen list

    date getVirusDate(stringList* virus);       //   given the virus name return the date of that virus
    date getVirusDate(string virus);            //   EMPTY DATE if that virus is not in that citizen list

    // GETTERS
    stringList* getVirusName() { return this->virusName; }
    char getVirusStatus() { return this->status; }
    date getDateVaccinated() { return this->dateVaccinated; }
    listStatus* getNext() { return this->next; }

    // SETTERS
    void setStatus(char status) { this->status = status; }
    void setDate(date d) { this->dateVaccinated = d; }
    void setDate(string d) { this->dateVaccinated.setAll(d); }


private:
    stringList* virusName;
    char status; // 'y' stands for "YES", 'n' for "NO"
    date dateVaccinated;
    listStatus* next;
};

class citizenRecord
    /* This class contains the credentials of a citizen together
    with a header for listStatus list*/
{
public:
    citizenRecord(int id, string name, string lastn, stringList* count, int ag, listStatus* stat);
    citizenRecord(int id, string name, string lastn, stringList* count, int ag, stringList* virus, char stat, string d);
    ~citizenRecord();

    void print();
    void printCredentials();

    int credentialsMatches(citizenRecord* citizen);

    // GETTERS
    int getID() { return this->citizenID; }
    string getFirstName() { return this->firstName; }
    string getLastName() { return this->lastName; }
    stringList* getCountry() { return this->country; }
    int getAge() { return this->age; }
    listStatus* getStatus() { return this->status; }

private:
    int citizenID;
    string firstName;
    string lastName;
    stringList* country;
    int age;
    listStatus* status;
};

#endif