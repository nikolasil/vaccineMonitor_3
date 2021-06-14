/*
    This file conatins the bloomFilter together with a list of bloomFilters.

    BloomFilter is just a class that has an array of chars.
    In order to manupulate the array as array of bits bitwise shifting is used
    to access the spesific bit and set it (1) or clear it (0).
*/

#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include "../stringList/stringList.h"

class bloomFilter
{
public:
    bloomFilter(int bloomSize);
    ~bloomFilter();

    void setBit(int k, int value);
    int getBit(int k);
    int getSize() { return this->bloomSize * sizeof(char) * 8; }
    void add(int number);
    int check(int number);
    char* getArray() { return this->array; };
    void print();
private:
    unsigned long djb2(unsigned char* str);
    unsigned long sdbm(unsigned char* str);
    unsigned long hash_i(unsigned char* str, int i);
    int bloomSize;
    char* array;
};

class bloomFilterList
{
public:
    bloomFilterList(int bloomSize);
    bloomFilterList(stringList* virus, int bloomSize);
    ~bloomFilterList();

    bloomFilterList* add(stringList* virus);
    bloomFilter* getBloom(stringList* virus);

private:
    int bloomSize;
    stringList* virus;
    bloomFilter* bloom;
    bloomFilterList* next;
};

#endif