#include <iostream>

#include "../../util.h"
#include "stringList.h"



stringList::stringList() : data("") {
    this->next == nullptr;
}

stringList::stringList(string d) : data(d) {
    this->next == nullptr;
}

stringList::~stringList()
{
    if (this->getNext() != nullptr)
        delete this->getNext();
}

stringList* stringList::add(string d)
{
    if (this->getString().compare("") == 0)
    {
        this->setString(d);
        return this;
    }
    stringList* new_node = new stringList(d);
    checkNew(new_node);
    new_node->setNext(this);
    return new_node;
}

stringList* stringList::remove(string d)
{
    stringList* temp = this;
    stringList* prev = nullptr;

    if (temp != nullptr && temp->getString() == d)
    {
        stringList* temp2 = temp->getNext();
        delete temp;
        return temp2;
    }
    else
    {
        while (temp != nullptr && temp->getString() != d)
        {
            prev = temp;
            temp = temp->getNext();
        }
        if (temp == nullptr)
            return this;

        prev->setNext(temp->getNext());
        delete temp;
        return this;
    }
}

stringList* stringList::search(string d)
{
    stringList* temp = this;
    while (temp != nullptr)
    {
        if (temp->getString().compare(d) == 0)
            return temp;

        temp = temp->getNext();
    }
    return temp;
}

void stringList::printData()
{
    cout << this->getString();
}

void stringList::print()
{
    stringList* temp = this;
    while (temp != nullptr)
    {
        temp->printData();
        cout << " ";
        temp = temp->getNext();
    }
    cout << endl;
}