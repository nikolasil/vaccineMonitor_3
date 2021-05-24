#include <iostream>
#include <string>

#include "../../util.h"
#include "statsList.h"



statsList::statsList() : country(""), virusName("") {
    this->next == NULL;
}

statsList::statsList(string c, string v, date dt, bool s) : country(c), virusName(v), stat(s) {
    this->d.setYear(dt.getYear());
    this->d.setMonth(dt.getMonth());
    this->d.setDay(dt.getDay());
    this->next == NULL;
}

statsList::~statsList()
{
    if (this->getNext() != NULL)
        delete this->getNext();
}

statsList* statsList::add(string c, string v, date dt, bool s)
{
    if (this->getCountry().compare("") == 0)
    {
        this->setCountry(c);
        this->setVirusName(v);
        this->setDate(dt);
        this->setStat(s);
        return this;
    }
    statsList* new_node = new statsList(c, v, dt, s);
    checkNew(new_node);
    new_node->setNext(this);
    return new_node;
}

void statsList::printNode()
{
    cout << this->getCountry() << " " << this->getVirusName() << " ";
    this->getDate().print();
    if (this->getStat())
        cout << " ACCEPTED" << endl;
    else
        cout << " REJECTED" << endl;
}

void statsList::print()
{
    statsList* temp = this;
    while (temp != NULL)
    {
        temp->printNode();
        temp = temp->getNext();
    }
}