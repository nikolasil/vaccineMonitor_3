/*
    This file contains the treeNode class.

    Is a self balanded tree that holds a pointer to a citizenRecord as data.
*/

#ifndef TREE_H
#define TREE_H

#include "../../citizenRecords/citizen.h"

using namespace std;

class treeNode
{
public:
    treeNode(citizenRecord* citizen);
    ~treeNode();

    void print(treeNode* node);

    treeNode* rightRotation(treeNode* y);
    treeNode* leftRotation(treeNode* x);

    treeNode* insert(treeNode* node, citizenRecord* citizen, citizenRecord** alreadyInTree, string* result, bool checkNO);
    treeNode* search(treeNode* root, int id);

    int getBalance();

    // GETTERS
    citizenRecord* getCitizen() { return this->citizen; }
    treeNode* getLeft() { return this->left; }
    treeNode* getRight() { return this->right; }
    int getKey() { return this->citizen->getID(); }
    int getBalanceHeight() { if (this == NULL) return 0; return this->balanceHeight; }

    // SETTERS
    void setCitizen(citizenRecord* citizen) { this->citizen = citizen; }
    void setLeft(treeNode* l) { this->left = l; }
    void setRight(treeNode* r) { this->right = r; }
    void setBalanceHeight(int b) { this->balanceHeight = b; }

private:
    treeNode* left;
    treeNode* right;
    citizenRecord* citizen;
    int balanceHeight;
};

#endif