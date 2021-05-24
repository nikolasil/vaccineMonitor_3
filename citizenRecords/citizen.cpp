#include <iostream>
#include <string>

#include "citizen.h"
#include "../util.h"
#include "../DataStructures/date/date.h"
#include "../DataStructures/stringList/stringList.h"


/*
- - - listStatus Methods Iimplemantation - - -
*/

listStatus::listStatus(stringList* virus, char s, string d) : dateVaccinated(d)
{
    this->virusName = virus;
    this->status = s;
    this->next = NULL;
}

listStatus::listStatus(stringList* virus, char s, date d) : dateVaccinated(d)
{
    this->virusName = virus;
    this->status = s;
    this->next = NULL;
}

listStatus::~listStatus()
{
    if (this->next != NULL)
        delete this->next;
}

void listStatus::print()
{
    listStatus* temp = this;
    do
    {
        cout << "[virusName: ";
        temp->virusName->printData();

        if (temp->status == 'y')
            cout << ", status: YES";
        else if (temp->status == 'n')
            cout << ", status: NO";

        date nullDate("", "", "");
        if (temp->dateVaccinated.compare(nullDate) != 0)
        {
            cout << " , dateVaccinated: ";
            temp->dateVaccinated.print();
        }
        cout << "]";
        temp = temp->next;
    } while (temp != NULL);
}

void listStatus::addStatus(stringList* virus, char s, date d)
{
    listStatus* temp = this;
    if (temp == NULL)
    {
        this->virusName = virus;
        this->status = s;
        this->dateVaccinated = d;
        return;
    }

    while (temp->next != NULL)
        temp = temp->next;

    listStatus* new_node = new listStatus(virus, s, d);
    checkNew(new_node);
    temp->next = new_node;
}

listStatus* listStatus::getNode(stringList* virus)
{
    listStatus* temp = this;
    do
    {
        if (temp->virusName->getString().compare(virus->getString()) == 0)
            return temp;

        temp = temp->next;
    } while (temp != NULL);
    return NULL;
}

listStatus* listStatus::getNode(string virus)
{
    listStatus* temp = this;
    do
    {
        if (temp->virusName->getString().compare(virus) == 0)
            return temp;

        temp = temp->next;
    } while (temp != NULL);
    return NULL;
}

char listStatus::getVirusStatus(stringList* virus)
{
    listStatus* temp = this;
    do
    {
        if (temp->virusName->getString().compare(virus->getString()) == 0)
            return temp->status;

        temp = temp->next;
    } while (temp != NULL);
    return '\0';
}

char listStatus::getVirusStatus(string virus)
{
    listStatus* temp = this;
    do
    {
        if (temp->virusName->getString().compare(virus) == 0)
            return temp->status;

        temp = temp->next;
    } while (temp != NULL);
    return '\0';
}

date listStatus::getVirusDate(stringList* virus)
{
    listStatus* temp = this;
    do
    {
        if (temp->virusName->getString().compare(virus->getString()) == 0)
            return temp->dateVaccinated;

        temp = temp->next;
    } while (temp != NULL);
    date nullDate("", "", "");
    return nullDate;
}

date listStatus::getVirusDate(string virus)
{
    listStatus* temp = this;
    do
    {
        if (temp->virusName->getString().compare(virus) == 0)
            return temp->dateVaccinated;

        temp = temp->next;
    } while (temp != NULL);
    date nullDate("", "", "");
    return nullDate;
}

/*
- - - citizenRecord Methods Iimplemantation - - -
*/

citizenRecord::citizenRecord(int id, string name, string lastn, stringList* count, int ag, listStatus* stat) : citizenID(id), firstName(name), lastName(lastn), country(count), age(ag), status(stat) {}

citizenRecord::citizenRecord(int id, string name, string lastn, stringList* count, int ag, stringList* virus, char stat, string d) : citizenID(id), firstName(name), lastName(lastn), country(count), age(ag)
{
    this->status = new listStatus(virus, stat, d);
    checkNew(this->status);
}

citizenRecord::~citizenRecord()
{
    delete this->status;
}

int citizenRecord::credentialsMatches(citizenRecord* citizen)
{
    if (this->getID() == citizen->getID() && this->getFirstName().compare(citizen->getFirstName()) == 0 && this->getLastName().compare(citizen->getLastName()) == 0 && this->getCountry()->getString().compare(citizen->getCountry()->getString()) == 0 && this->getAge() == citizen->getAge())
        return 1;
    return 0;
}

void citizenRecord::print()
{
    cout << "citizenID: " << this->citizenID;
    cout << ", firstName: " << this->firstName;
    cout << ", lastName: " << this->lastName;
    cout << ", country: ";
    this->country->printData();
    cout << ", age: " << this->age << " ";
    this->status->print();
    cout << endl;
}

void citizenRecord::printCredentials()
{
    cout << "citizenID: " << this->citizenID;
    cout << ", firstName: " << this->firstName;
    cout << ", lastName: " << this->lastName;
    cout << ", country: ";
    this->country->printData();
    cout << ", age: " << this->age << " ";
    cout << endl;
}