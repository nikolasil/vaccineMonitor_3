/*
    This files contains the skipListNode, skipListLevel, skipList and skipList_List classes.
*/

#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <limits>
#include "../../citizenRecords/citizen.h"
#include "../stringList/stringList.h"

#define POS_INF std ::numeric_limits<int>::max()
#define NEG_INF std ::numeric_limits<int>::min()
#define maxLevels 9

class skipListNode
{
public:
    skipListNode(citizenRecord* citizen);
    skipListNode(skipListNode* node);
    ~skipListNode();
    void destroy();

    void printIds();
    void printCitizens();

    void add(skipListNode* new_node);

    // GETTERS
    skipListNode* getNext() { return this->next; }
    skipListNode* getDown() { return this->down; }
    int getId() { if (this->citizen != NULL) return this->citizen->getID(); else if (this->next == NULL) return POS_INF; return NEG_INF; }
    citizenRecord* getCitizen() { return this->citizen; }

    // SETTERS
    void setNext(skipListNode* temp) { this->next = temp; }
    void setDown(skipListNode* temp) { this->down = temp; }
    void setCitizen(citizenRecord* citizen) { this->citizen = citizen; }


private:
    citizenRecord* citizen;
    skipListNode* next;
    skipListNode* down;
};

class skipListLevel
{
public:
    skipListLevel(skipListLevel* prevLevel, int l);
    skipListLevel();
    ~skipListLevel();

    void printIds();
    void printCitizens();

    skipListLevel* getDownLevel() { return this->downLevel; }
    int getMyLevel() { return this->myLevel; }
    skipListNode* getList() { return this->list; }
    skipListNode* getPosInf() { return this->pos_inf; }
    skipListNode* getNegInf() { return this->list; }

    void setDownLevel(skipListLevel* down) { this->downLevel = down; }
    void setList(skipListNode* l) { this->list = l; }
    void setPosInf(skipListNode* p) { this->pos_inf = p; }
    void setNegInf(skipListNode* n) { this->list = n; }
    void setMyLevel(int l) { this->myLevel = l; }

private:
    int myLevel;
    skipListNode* list;
    skipListNode* pos_inf;
    skipListLevel* downLevel;
};

class skipList
{
public:
    skipList();
    ~skipList();

    void printIds();
    void printFloor();

    bool isEmpty();
    int coinFlip();

    void add(int id, citizenRecord* citizen);
    void remove(int id);
    skipListNode* search(int id, char top_bottom);

    // GETTERS
    skipListLevel* getCeiling() { return this->ceiling; }
    skipListLevel* getFloor() { return this->floor; }
    // SETTERS
    void setCeiling(skipListLevel* c) { this->ceiling = c; }
    void setFloor(skipListLevel* f) { this->floor = f; }

private:
    skipListLevel* ceiling;
    skipListLevel* floor;
};

class skipList_List
{
public:
    skipList_List();
    skipList_List(stringList* virus);
    ~skipList_List();

    skipList_List* add(stringList* virus);

    skipList* getVaccinated(stringList* virus);
    skipList* getNotVaccinated(stringList* virus);
    void setNotVaccinated(skipList* l, stringList* virus);
    void setVaccinated(skipList* l, stringList* virus);

    // GETTERS
    stringList* getVirus() { return this->virus; }
    skipList* getVaccinated() { return this->vaccinated; }
    skipList* getNotVaccinated() { return this->notVaccinated; }
    skipList_List* getNext() { return this->next; }
    // SETTERS
    void setVirus(stringList* v) { this->virus = v; }
    void setVaccinated(skipList* l) { this->vaccinated = l; }
    void setNotVaccinated(skipList* l) { this->notVaccinated = l; }
    void setNext(skipList_List* n) { this->next = n; }

private:
    stringList* virus;
    skipList* vaccinated;
    skipList* notVaccinated;
    skipList_List* next;
};

#endif