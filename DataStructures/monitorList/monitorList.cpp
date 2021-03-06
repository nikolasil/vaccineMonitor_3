#include <iostream>

#include "../../util.h"
#include "monitorList.h"

monitorList::monitorList() {}

monitorList::monitorList(int pid, int id) : pid(pid), id(id) {
    this->next = nullptr;
}

monitorList::~monitorList()
{
    if (this->getNext() != nullptr)
        delete this->getNext();
}

monitorList* monitorList::add(int pid, int id)
{
    monitorList* temp = this->getPIDNode(id);
    if (temp != nullptr) {
        // already exists then update the pid

        // cout << "Old pid " << temp->getPID() << endl;
        temp->setPID(pid);
        cout << "New pid " << temp->getPID() << endl;
        return temp;
    }
    temp = this;
    while (temp->getNext() != nullptr)
        temp = temp->getNext();

    monitorList* new_node = new monitorList(pid, id);
    checkNew(new_node);
    temp->setNext(new_node);
    return new_node;
}

monitorList* monitorList::addFD(int m, int s)
{
    monitorList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();
    // cout << "old fd " << temp->getReadFD() << " " << temp->getWriteFD() << endl;
    temp->setSocketFD(s);
    // cout << "new fd " << temp->getReadFD() << " " << temp->getWriteFD() << endl;
    return temp;
}

int monitorList::getSocketFD(int m) {
    monitorList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();

    return temp->getSocketFD();
}

int monitorList::getPID(int m) {
    monitorList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();

    return temp->getPID();
}

monitorList* monitorList::getPIDNode(int m) {
    monitorList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();

    return temp;
}

int monitorList::getID(int pid) {
    monitorList* temp = this;
    while (temp != nullptr) {
        if (temp->getPID() == pid)
            return temp->getID();
        temp = temp->getNext();
    }
    return -1;
}

void monitorList::print() {
    monitorList* temp = this;
    while (temp != nullptr) {
        cout << temp->id << " " << temp->pid << endl;
        temp = temp->getNext();
    }
}