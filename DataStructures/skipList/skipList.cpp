#include <iostream>

#include "skipList.h"
#include "../../citizenRecords/citizen.h"
#include "../stringList/stringList.h"
#include "../../util.h"

using namespace std;

skipListNode::skipListNode(citizenRecord* citizen) : citizen(citizen)
{
    this->setNext(NULL);
    this->setDown(NULL);
}

skipListNode::skipListNode(skipListNode* node)
{
    this->setCitizen(node->getCitizen());
    this->setNext(NULL);
    this->setDown(node);
}

skipListNode::~skipListNode()
{
}

void skipListNode::destroy()
{
    skipListNode* temp = this->getNext();
    while (temp != NULL)
    {
        skipListNode* next = temp->getNext();
        delete temp;
        temp = next;
    }
    delete this;
}

void skipListNode::printIds()
{
    skipListNode* temp = this->getNext();
    while (temp != NULL)
    {
        cout << temp->getId() << " ";
        temp = temp->getNext();
        if (temp->getNext() == NULL)
            break;
    }
    cout << endl;
}

void skipListNode::printCitizens()
{
    skipListNode* temp = this->getNext();
    while (temp != NULL)
    {
        temp->getCitizen()->printCredentials();
        temp = temp->getNext();
        if (temp->getNext() == NULL)
            break;
    }
    cout << endl;
}

void skipListNode::add(skipListNode* new_node)
{
    skipListNode* curr = this;
    while (curr->getNext() != NULL && curr->getNext()->getId() < new_node->getId())
        curr = curr->getNext();

    new_node->setNext(curr->getNext());
    curr->setNext(new_node);
}

// -- skipListLevel methods implemantation --

skipListLevel::skipListLevel(skipListLevel* prevLevel, int l) : myLevel(l)
/* when we make a new level we must make the negative and positive infinity points
to the down levels negative and positive infinity  */
{
    this->setList(new skipListNode((citizenRecord*)NULL));
    checkNew(this->getList());
    this->setPosInf(new skipListNode((citizenRecord*)NULL));
    checkNew(this->getPosInf());
    this->getList()->setNext(this->getPosInf());

    this->getList()->setDown(prevLevel->getNegInf());
    this->getPosInf()->setDown(prevLevel->getPosInf());

    this->setDownLevel(prevLevel);
}

skipListLevel::skipListLevel() : myLevel(0) /* this costructor in beeing called only for the 0 level */
{
    this->setList(new skipListNode((citizenRecord*)NULL));
    checkNew(this->getList());
    this->setPosInf(new skipListNode((citizenRecord*)NULL));
    checkNew(this->getPosInf());
    this->getList()->setNext(this->getPosInf());

    this->setDownLevel(NULL);
}

skipListLevel::~skipListLevel() {}

void skipListLevel::printIds() /* print only the list of the level */
{
    this->getList()->printIds();
}

void skipListLevel::printCitizens() /* print only the list of the level */
{
    this->getList()->printCitizens();
}

// skipList methods implemantation

int skipList::coinFlip() /* returns 0 or 1 with 50% change each */
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((time_t)ts.tv_nsec); // tv_nsec to change the value with nanoseconds so the function dont return the same thing every time

    return rand() % 2;
}

skipList::skipList() /* create the skip list with level 0 that has negative infinity
                        and posotive infinty nodes */
{
    this->setFloor(new skipListLevel());
    checkNew(this->getFloor());
    this->setCeiling(this->floor);
}

skipList::~skipList() /* delete the all the skip list levels and nodes */
{
    skipListLevel* temp = this->getCeiling();
    while (temp != NULL)
    {
        temp->getList()->destroy();
        skipListLevel* next = temp->getDownLevel();
        delete temp;
        temp = next;
    }
}

void skipList::add(int id, citizenRecord* citizen) /* add the id in the skip list
                            if it is the same id no insertion will happen*/
{
    skipListLevel* currLevel = this->getCeiling();
    skipListNode* currNode = currLevel->getNegInf();
    skipListNode* nextNode = currNode->getNext();

    skipListNode* prevCreated = NULL;
    int create = 0;
    int heightOfNewNode = 0;
    while (this->coinFlip() && (this->getCeiling()->getMyLevel() >= heightOfNewNode) && this->getCeiling()->getMyLevel() < maxLevels - 1)
        heightOfNewNode++;

    while (1)
    {
        if (id < nextNode->getId())
        {
            if (currLevel->getMyLevel() <= heightOfNewNode)
            { // create a node
                if (create)
                {
                    skipListNode* downNode = new skipListNode(citizen);
                    checkNew(downNode);
                    prevCreated->setDown(downNode);
                    currNode->setNext(downNode);
                    downNode->setNext(nextNode);
                    prevCreated = downNode;
                }
                else if (!create) // if it is the first time
                {
                    prevCreated = new skipListNode(citizen);
                    checkNew(prevCreated);
                    currNode->setNext(prevCreated);
                    prevCreated->setNext(nextNode);
                    if (currLevel == this->getCeiling() && heightOfNewNode == this->getCeiling()->getMyLevel() + 1)
                    { // make a new level
                        skipListLevel* newLevel = new skipListLevel(this->getCeiling(), this->getCeiling()->getMyLevel() + 1);
                        checkNew(newLevel);
                        this->setCeiling(newLevel);
                        skipListNode* newN = new skipListNode(prevCreated);
                        checkNew(newN);
                        this->getCeiling()->getList()->add(newN);
                    }
                    create = 1;
                }
            }

            if (currNode->getDown() != NULL)
            { // if there is a down level
                // step down
                currNode = currNode->getDown();
                nextNode = currNode->getNext();
                currLevel = currLevel->getDownLevel();
            }
            else
            {
                if (!create)
                {
                    prevCreated = new skipListNode(citizen);
                    checkNew(prevCreated);
                    currNode->setNext(prevCreated);
                    prevCreated->setNext(nextNode);
                }
                return;
            }
        }
        else if (id > nextNode->getId())
        {
            currNode = nextNode;
            nextNode = nextNode->getNext();
        }
        else if (id == nextNode->getId())
            return;
    }
}

void skipList::remove(int id)
{
    skipListLevel* currLevel = this->getCeiling();
    skipListNode* currNode = currLevel->getNegInf();
    skipListNode* nextNode = currNode->getNext();

    while (1)
    {
        if (id < nextNode->getId())
        {
            if (currNode->getDown() != NULL)
            { // if there is a down level
                // step down
                currNode = currNode->getDown();
                nextNode = currNode->getNext();
                currLevel = currLevel->getDownLevel();
            }
            else
                return;
        }
        else if (id > nextNode->getId())
        {
            currNode = nextNode;
            nextNode = nextNode->getNext();
        }
        else
        {
            int deleteLevel = 0;
            if (currNode == this->getCeiling()->getNegInf() && nextNode->getNext() == this->getCeiling()->getPosInf())
                deleteLevel = 1;

            skipListNode* toDelete = nextNode;
            currNode->setNext(nextNode->getNext());
            nextNode = toDelete->getDown();
            delete toDelete;
            if (deleteLevel && this->getCeiling() != this->getFloor())
            {
                skipListLevel* temp = this->getCeiling()->getDownLevel();
                this->getCeiling()->getList()->destroy();
                delete this->getCeiling();
                this->setCeiling(temp);
            }
            if (nextNode != NULL)
            {
                currNode = currNode->getDown();
                while (1)
                {
                    if (currNode->getNext() == nextNode)
                        break;

                    currNode = currNode->getNext();
                }
                currLevel = currLevel->getDownLevel();
            }
            else
                return;
        }
    }
}

skipListNode* skipList::search(int id, char top_bottom = 't')
{
    skipListLevel* currLevel = this->getCeiling();
    skipListNode* currNode = currLevel->getNegInf();
    skipListNode* nextNode = currNode->getNext();

    while (1)
    {
        if (id < nextNode->getId())
        {
            if (currNode->getDown() != NULL)
            { // if there is a down level
                // step down
                currNode = currNode->getDown();
                nextNode = currNode->getNext();
                currLevel = currLevel->getDownLevel();
            }
            else
                return NULL;
        }
        else if (id > nextNode->getId())
        {
            currNode = nextNode;
            nextNode = nextNode->getNext();
        }
        else
        {
            if (top_bottom == 'b')
            {
                while (1)
                {
                    if (nextNode->getDown() == NULL)
                        break;

                    nextNode = nextNode->getDown();
                }
            }
            return nextNode;
        }
    }
}

bool skipList::isEmpty()
{
    if (this->floor->getNegInf() == this->floor->getPosInf())
        return 1;

    return 0;
}

void skipList::printIds() /* print all the levels one by one */
{
    skipListLevel* temp = this->getCeiling();
    while (temp != NULL)
    {
        temp->printIds();
        temp = temp->getDownLevel();
    }
}

void skipList::printFloor() /* print all the levels one by one */
{
    this->getFloor()->printCitizens();
}

// skipList methods implemantation

skipList_List::skipList_List()
{
    this->setVirus(NULL);
    this->setVaccinated(NULL);
    this->setNotVaccinated(NULL);
    this->setNext(NULL);
}

skipList_List::skipList_List(stringList* virus)
{
    this->setVirus(virus);
    this->setVaccinated(new skipList());
    checkNew(this->getVaccinated());
    this->setNotVaccinated(new skipList());
    checkNew(this->getNotVaccinated());
    this->setNext(NULL);
}

skipList_List::~skipList_List()
{
    if (this->getVaccinated() != NULL)
        delete this->getVaccinated();

    if (this->getNotVaccinated() != NULL)
        delete this->getNotVaccinated();

    if (this->getNext() != NULL)
        delete this->getNext();

}

skipList_List* skipList_List::add(stringList* virus)
{
    if (this->getVirus() == NULL)
    {
        this->setVirus(virus);
        this->setVaccinated(new skipList());
        checkNew(this->getVaccinated());
        this->setNotVaccinated(new skipList());
        checkNew(this->getNotVaccinated());
        return this;
    }
    skipList_List* new_node = new skipList_List(virus);
    checkNew(new_node);
    new_node->setNext(this);
    return new_node;
}

skipList* skipList_List::getVaccinated(stringList* virus)
{
    skipList_List* temp = this;
    while (temp != NULL)
    {
        if (temp->getVirus()->getString().compare(virus->getString()) == 0)
            return temp->getVaccinated();

        temp = temp->getNext();
    }
    return NULL;
}

skipList* skipList_List::getNotVaccinated(stringList* virus)
{
    skipList_List* temp = this;
    while (temp != NULL)
    {
        if (temp->getVirus()->getString().compare(virus->getString()) == 0)
            return temp->getNotVaccinated();

        temp = temp->getNext();
    }
    return NULL;
}

void skipList_List::setVaccinated(skipList* l, stringList* virus)
{
    skipList_List* temp = this;
    while (temp != NULL)
    {
        if (temp->getVirus()->getString().compare(virus->getString()) == 0)
        {
            temp->setVaccinated(l);
            return;
        }
        temp = temp->getNext();
    }
    return;
}

void skipList_List::setNotVaccinated(skipList* l, stringList* virus)
{
    skipList_List* temp = this;
    while (temp != NULL)
    {
        if (temp->getVirus()->getString().compare(virus->getString()) == 0)
        {
            temp->setNotVaccinated(l);
            return;
        }
        temp = temp->getNext();
    }
    return;
}