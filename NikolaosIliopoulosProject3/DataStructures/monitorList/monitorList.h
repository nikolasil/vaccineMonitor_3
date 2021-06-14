/*
    This files contains the monitorList class that is
    used to store the virus and countries names to avoid country duplication.

    Every time a country name or a virus name is needed a pointer to the node is made.
*/

#ifndef monitorList_H
#define monitorList_H

#include <string>

using namespace std;

class monitorList
{
public:
    monitorList();
    monitorList(int pid, int id);
    ~monitorList();

    monitorList* add(int pid, int id);
    monitorList* addFD(int m, int s);
    int getSocketFD(int m);
    int getPID(int m);
    monitorList* getPIDNode(int m);
    int getID(int pid);

    // GETTERS
    int getSocketFD() { return this->socketFD; }
    int getPID() { return this->pid; }
    int getID() { return this->id; }
    monitorList* getNext() { return this->next; }

    // SETTERS
    void setSocketFD(int s) { this->socketFD = s; }
    void setPID(int p) { this->pid = p; }
    void setID(int i) { this->id = i; }
    void setNext(monitorList* n) { this->next = n; }

    void print();
private:
    int socketFD;
    int pid;
    int id;

    monitorList* next;
};

#endif