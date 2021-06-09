#include <iostream>

#include "bloomFilter.h"
#include "../stringList/stringList.h"
#include "../../util.h"

using namespace std;

/*
- - - bloomFilter Methods Iimplemantation - - -
*/

bloomFilter::bloomFilter(int bloomSize)
{
    this->bloomSize = bloomSize / sizeof(char);
    this->array = new char[this->bloomSize];
    checkNew(this->array);
    for (int i = 0; i < bloomSize; i++)
        this->array[i] = 0;
}

bloomFilter::~bloomFilter()
{
    if (this->array != nullptr)
        delete this->array;
}

void bloomFilter::setBit(int k, int value)
{
    int position = (k / (sizeof(char) * 8));
    int shift = (k % (sizeof(char) * 8));
    if (value == 1)
        this->array[position] = this->array[position] | (1 << shift);
    else if (value == 0)
        this->array[position] = this->array[position] & ~(1 << shift);
}

int bloomFilter::getBit(int k)
{
    int position = (k / (sizeof(char) * 8));
    int shift = (k % (sizeof(char) * 8));
    if (this->array[position] & (1 << shift))
        return 1;
    return 0;
}

unsigned long bloomFilter::djb2(unsigned char* str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

unsigned long bloomFilter::sdbm(unsigned char* str)
{
    unsigned long hash = 0;
    int c;

    while ((c = *str++))
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

unsigned long bloomFilter::hash_i(unsigned char* str, int i)
{
    return this->djb2(str) + i * this->sdbm(str) + i * i;
}

void bloomFilter::add(int number)
{
    char numberstring[4];
    snprintf(numberstring, 5, "%d", number);
    unsigned char unsignedNumber[4];
    unsignedNumber[0] = numberstring[0];
    unsignedNumber[1] = numberstring[1];
    unsignedNumber[2] = numberstring[2];
    unsignedNumber[3] = numberstring[3];
    int K = 16;
    for (int i = 0; i < K; i++)
        this->setBit(this->hash_i(unsignedNumber, i) % this->getSize(), 1);

}

int bloomFilter::check(int number)
{
    char numberstring[4];
    snprintf(numberstring, 4, "%d", number);
    unsigned char unsignedNumber[4];
    unsignedNumber[0] = numberstring[0];
    unsignedNumber[1] = numberstring[1];
    unsignedNumber[2] = numberstring[2];
    unsignedNumber[3] = numberstring[3];
    int K = 16;
    int flag = 1;
    for (int i = 0; i < K; i++)
    {
        if (!this->getBit(this->hash_i(unsignedNumber, i) % this->getSize()))
        {
            flag = 0;
            break;
        }
    }
    return flag;
}

void bloomFilter::print() {
    for (int i = 0; i < this->getSize(); i++)
        if (this->getBit(i) == 1)
            cout << 1;
        else
            cout << 0;
    cout << endl;
}

/*
- - - bloomFilterList Methods Iimplemantation - - -
*/

bloomFilterList::bloomFilterList(int bloomSize) : bloomSize(bloomSize)
{
    this->next = nullptr;
    this->bloom = nullptr;
    this->virus = nullptr;
}

bloomFilterList::bloomFilterList(stringList* virus, int bloomSize) : bloomSize(bloomSize), virus(virus)
{
    this->bloom = new bloomFilter(this->bloomSize);
    checkNew(this->bloom);
    this->next = nullptr;
}

bloomFilterList::~bloomFilterList()
{
    if (this->bloom != nullptr)
        delete this->bloom;

    if (this->next != nullptr)
        delete this->next;

}

bloomFilterList* bloomFilterList::add(stringList* virus)
{
    cout << "Adding to bloom filter" << endl;
    if (this->virus == nullptr)
    {
        cout << "in if" << endl;
        this->virus = virus;
        this->bloom = new bloomFilter(this->bloomSize);
        checkNew(this->bloom);
        cout << "done add0" << endl;
        return this;
    }
    cout << "out if " << this->bloomSize << endl;
    bloomFilterList* new_node = new bloomFilterList(virus, this->bloomSize);
    cout << "after new" << endl;
    checkNew(new_node);
    cout << "check new" << endl;
    new_node->next = this;
    cout << "set next" << endl;
    cout << "done add1" << endl;
    return new_node;
}

bloomFilter* bloomFilterList::getBloom(stringList* virus)
{
    bloomFilterList* temp = this;
    while (temp != nullptr)
    {
        if (temp->virus->getString().compare(virus->getString()) == 0)
            return temp->bloom;

        temp = temp->next;
    }
    return nullptr;
}