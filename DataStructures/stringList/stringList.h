/*
    This files contains the stringList class that is
    used to store the virus and countries names to avoid data duplication.

    Every time a country name or a virus name is needed a pointer to the node is made.
*/

#ifndef STRINGLIST_H
#define STRINGLIST_H

#include <string>

using namespace std;

class stringList
{
public:
    stringList();
    stringList(string d);
    ~stringList();

    void printData(); // print only this node
    void print();     // print this and all the next nodes

    stringList* add(string d);
    stringList* remove(string d);
    stringList* search(string d);

    // GETTERS
    string getString() { return this->data; }
    stringList* getNext() { return this->next; }

    // SETTERS
    void setString(string s) { this->data = s; }
    void setNext(stringList* n) { this->next = n; }

private:
    string data;
    stringList* next;
};

#endif