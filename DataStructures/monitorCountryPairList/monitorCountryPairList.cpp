#include <iostream>
#include <string>

#include "../../util.h"
#include "monitorCountryPairList.h"


monitorCountryPairList::monitorCountryPairList() : country("") {}

monitorCountryPairList::monitorCountryPairList(string d, int m) : country(d), monitor(m) {}

monitorCountryPairList::~monitorCountryPairList()
{
    if (this->getNext() != NULL)
        delete this->getNext();
}

monitorCountryPairList* monitorCountryPairList::add(string d, int m)
{
    monitorCountryPairList* new_node = new monitorCountryPairList(d, m);
    checkNew(new_node);
    new_node->setNext(this);
    return new_node;
}

monitorCountryPairList* monitorCountryPairList::search(string d)
{
    monitorCountryPairList* temp = this;
    while (temp != NULL)
    {
        if (temp->getCountry().compare(d) == 0)
            return temp;

        temp = temp->getNext();
    }
    return temp;
}

void monitorCountryPairList::print()
{
    monitorCountryPairList* temp = this;
    while (temp != NULL)
    {
        cout << temp->country << " is assinged to monitor " << temp->monitor << endl;;
        temp = temp->getNext();
    }
    cout << endl;
}