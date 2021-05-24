#include <iostream>

#include "tree.h"
#include "../../citizenRecords/citizen.h"
#include "../../util.h"


/*
- - - treeNode Methods Iimplemantation - - -
*/

treeNode::treeNode(citizenRecord* citizen)
{
    this->setCitizen(citizen);
    this->setLeft(NULL);
    this->setRight(NULL);
    this->setBalanceHeight(1);
}

treeNode::~treeNode()
{
    delete this->getCitizen();
    delete this->getLeft();
    delete this->getRight();
}

treeNode* treeNode::rightRotation(treeNode* y)
{
    treeNode* x = y->getLeft();
    treeNode* z = x->getRight();

    x->setRight(y);
    y->setLeft(z);

    y->setBalanceHeight(max(y->getLeft()->getBalanceHeight(), y->getRight()->getBalanceHeight()) + 1);
    x->setBalanceHeight(max(x->getLeft()->getBalanceHeight(), x->getRight()->getBalanceHeight()) + 1);

    return x;
}

treeNode* treeNode::leftRotation(treeNode* x)
{
    treeNode* y = x->getRight();
    treeNode* z = y->getLeft();

    y->setLeft(x);
    x->setRight(z);

    x->setBalanceHeight(max(x->getLeft()->getBalanceHeight(), x->getRight()->getBalanceHeight()) + 1);
    y->setBalanceHeight(max(y->getLeft()->getBalanceHeight(), y->getRight()->getBalanceHeight()) + 1);

    return y;
}

treeNode* treeNode::insert(treeNode* node, citizenRecord* citizen, citizenRecord** alreadyInTree, string* result, bool checkNO)
{
    if (node == NULL)
    {
        *result = "NEW CITIZEN";
        return (new treeNode(citizen));
    }

    if (citizen->getID() < node->getCitizen()->getID()) // smaller GO LEFT
        node->setLeft(insert(node->getLeft(), citizen, alreadyInTree, result, checkNO));

    else if (citizen->getID() > node->getCitizen()->getID()) // bigger GO RIGHT
        node->setRight(insert(node->getRight(), citizen, alreadyInTree, result, checkNO));

    else if (citizen->getID() == node->getCitizen()->getID()) // equal
    {
        *alreadyInTree = node->getCitizen();
        if (!node->getCitizen()->credentialsMatches(citizen))
        {
            *result = "WRONG CREDENTIALS";
            delete citizen;
            return node;
        }
        char oldStatus = node->getCitizen()->getStatus()->getVirusStatus(citizen->getStatus()->getVirusName());
        if (checkNO)
        {
            char newStatus = citizen->getStatus()->getVirusStatus();
            if (oldStatus == '\0')
            {
                node->getCitizen()->getStatus()->addStatus(citizen->getStatus()->getVirusName(), citizen->getStatus()->getVirusStatus(), citizen->getStatus()->getDateVaccinated());
                *result = "VIRUS ADDED TO CITIZEN";
            }
            else if (oldStatus == 'n' && newStatus == 'n')
                *result = "OLD NO NEW NO";

            else if (oldStatus == 'n' && newStatus == 'y')
            {
                listStatus* virusStatusNode = node->getCitizen()->getStatus()->getNode(citizen->getStatus()->getVirusName());
                virusStatusNode->setStatus(citizen->getStatus()->getVirusStatus());
                virusStatusNode->setDate(citizen->getStatus()->getDateVaccinated());
                *result = "OLD NO NEW YES";
            }
            else if (oldStatus == 'y' && newStatus == 'n')
                *result = "OLD YES NEW NO";

            else if (oldStatus == 'y' && newStatus == 'y')
                *result = "OLD YES NEW YES";
        }
        else
        {
            if (oldStatus == '\0')
            {
                node->getCitizen()->getStatus()->addStatus(citizen->getStatus()->getVirusName(), citizen->getStatus()->getVirusStatus(), citizen->getStatus()->getDateVaccinated());
                *result = "VIRUS ADDED TO CITIZEN";
            }
            else *result = "VIRUS DUPLICATE";
        }
        delete citizen;
        return node;
    }

    // update balance height of node
    node->setBalanceHeight(max(node->getLeft()->getBalanceHeight(), node->getRight()->getBalanceHeight()) + 1);

    if (node->getBalance() > 1 && citizen->getID() < node->getLeft()->getCitizen()->getID())
        return rightRotation(node);

    if (node->getBalance() < -1 && citizen->getID() > node->getRight()->getCitizen()->getID())
        return leftRotation(node);

    if (node->getBalance() > 1 && citizen->getID() > node->getLeft()->getCitizen()->getID())
    {
        node->setLeft(leftRotation(node->getLeft()));
        return rightRotation(node);
    }
    if (node->getBalance() < -1 && citizen->getID() < node->getRight()->getCitizen()->getID())
    {
        node->setRight(rightRotation(node->getRight()));
        return leftRotation(node);
    }
    return node;
}

treeNode* treeNode::search(treeNode* root, int key)
{
    if (root == NULL || root->getKey() == key)
        return root;

    if (root->getKey() < key)
        return search(root->getRight(), key);

    return search(root->getLeft(), key);
}

int treeNode::getBalance()
{
    if (this == NULL)
        return 0;
    return this->getLeft()->getBalanceHeight() - this->getRight()->getBalanceHeight();
}