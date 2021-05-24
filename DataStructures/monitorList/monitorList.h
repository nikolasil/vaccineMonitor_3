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
    monitorList* addFD(int m, int r, int w);
    int getReadFifo(int m);
    int getWriteFifo(int m);
    int getPID(int m);
    monitorList* getPIDNode(int m);
    int getID(int pid);

    // GETTERS
    int getReadFD() { return this->readFD; }
    int getWriteFD() { return this->writeFD; }
    int getPID() { return this->pid; }
    int getID() { return this->id; }
    monitorList* getNext() { return this->next; }

    // SETTERS
    void setReadFD(int r) { this->readFD = r; }
    void setWriteFD(int w) { this->writeFD = w; }
    void setPID(int p) { this->pid = p; }
    void setID(int i) { this->id = i; }
    void setNext(monitorList* n) { this->next = n; }

    void print();
private:
    int readFD;
    int writeFD;
    int pid;
    int id;

    monitorList* next;
};

#endif