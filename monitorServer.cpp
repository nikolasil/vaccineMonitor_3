#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <cstdlib>
#include <pthread.h>

#include "monitorServer.h"
#include "cyclicBuffer.h"
#include "DataStructures/stringList/stringList.h"
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/skipList/skipList.h"
#include "DataStructures/binaryAvlTree/tree.h"
#include "citizenRecords/citizen.h"
#include "util.h"

using namespace std;

monitorServer monitor = monitorServer();

monitorServer::~monitorServer() {}

// void monitorServer::suicide() {
//     if (this->tree != nullptr)
//         delete this->tree;
//     if (this->skipLists != nullptr)
//         delete this->skipLists;
//     if (this->blooms != nullptr)
//         delete this->blooms;
//     if (this->viruses != nullptr)
//         delete this->viruses;
//     if (this->countries != nullptr)
//         delete this->countries;
//     if (this->filesReaded != nullptr)
//         delete this->filesReaded;
//     if (this->command != nullptr)
//         delete[] this->command;

//     cout << "monitorServer " << getpid() << " Terminated" << endl;
//     exit(1);
// }

// void monitorServer::waitForCommands() {
//     while (1)
//     {
//         // cout << "Waiting for commands " << this->id << endl;
//         string input = receiveStr();
//         int length;
//         this->command = readString(input, &length);
//         if (length > 0)
//         {
//             if (command[0].compare("/travelRequest") == 0)
//                 this->travelRequest(command, length);

//             if (command[0].compare("/searchVaccinationStatus") == 0)
//                 this->searchVaccinationStatus(command, length);

//             else
//                 cout << "Invalid command!" << endl;
//         }
//         if (command != nullptr)
//             delete[] command;
//     }
// }

// void monitorServer::travelRequest(string* arguments, int length) {
//     id = stoi(arguments[1]);
//     date checkDate = date((arguments[2]));
//     string countryFrom = arguments[3];
//     string countryTo = arguments[4];
//     string virusName = arguments[5];

//     stringList* v = this->viruses->search(virusName);
//     if (v != nullptr)
//     {
//         skipList* l;
//         l = skipLists->getVaccinated(v);
//         skipListNode* n = l->search(id, 't');

//         if (n == nullptr) {
//             sendStr("NO");
//             f++;
//         }
//         else {
//             sendStr("YES " + n->getCitizen()->getStatus()->getVirusDate(v).getConcatenate());
//             t++;
//         }
//     }
//     else {
//         sendStr("NO");
//         f++;
//     }
// }

// void monitorServer::searchVaccinationStatus(string* arguments, int length) {
//     id = stoi(arguments[1]);
//     treeNode* citizen = this->tree->search(this->tree, id);
//     if (citizen != nullptr) {
//         int end = 1;
//         if (write(writeFD, &end, sizeof(int)) == -1)
//             if (errno != 4)
//                 cout << "Error in writting to_tranfer with errno=" << errno << endl;
//         string credentials = arguments[1] + " " + citizen->getCitizen()->getFirstName();
//         credentials.append(" " + citizen->getCitizen()->getLastName());
//         credentials.append(" " + citizen->getCitizen()->getCountry()->getString());
//         sendStr(credentials);

//         string age = "AGE ";
//         age.append(to_string(citizen->getCitizen()->getAge()));
//         sendStr(age);

//         listStatus* status = citizen->getCitizen()->getStatus();
//         while (status != nullptr) {
//             // cout << "1" << endl;
//             string virusStatus = "";
//             virusStatus.append(status->getVirusName()->getString());
//             if (status->getVirusStatus() == 'y') {
//                 virusStatus.append(" VACCINATED ON ");
//                 virusStatus.append(status->getDateVaccinated().getConcatenate());
//             }
//             else {
//                 virusStatus.append(" NOT YET VACCINATED");
//             }
//             sendStr(virusStatus);
//             status = status->getNext();
//         }
//         end = -1;
//         if (write(writeFD, &end, sizeof(int)) == -1)
//             if (errno != 4)
//                 cout << "Error in writting to_tranfer with errno=" << errno << endl;
//     }
//     else {
//         int end = -1;
//         if (write(writeFD, &end, sizeof(int)) == -1)
//             if (errno != 4)
//                 cout << "Error in writting to_tranfer with errno=" << errno << endl;
//     }
// }

// void monitorServer::makeLogFile() {
//     int pid = getpid();
//     ofstream logfile("logfiles/log_file." + to_string(pid));
//     stringList* temp = this->countries;
//     while (temp != nullptr) {
//         logfile << temp->getString() << endl;
//         temp = temp->getNext();
//     }
//     logfile << "TOTAL TRAVEL REQUESTS " << this->t + this->f << endl;
//     logfile << "ACCEPTED " << this->t << endl;
//     logfile << "REJECTED " << this->f << endl;
//     logfile.close();
// }

monitorServer::monitorServer() {}

void monitorServer::start(int p, int t, int sb, int cb, int bloom, char** paths, int numPaths) {
    cout << "Monitor port=" << p << ", threads=" << t << ", sb=" << sb << ", cb=" << cb << ", bloom=" << bloom << endl;
    this->port = p;
    this->numThreads = t;
    this->socketBufferSize = sb;
    this->cyclicBufferSize = cb;
    this->bloomSize = bloom;
    this->argPaths = paths;
    this->argNumPaths = numPaths;

    this->tree = nullptr;
    this->countries = new stringList();
    checkNew(this->countries);

    this->viruses = new stringList();
    checkNew(this->viruses);

    this->filesReaded = new stringList();
    checkNew(this->filesReaded);

    this->skipLists = new skipList_List();
    checkNew(this->skipLists);

    this->blooms = new bloomFilterList(this->bloomSize);
    checkNew(this->blooms);

    this->buff = new cyclicBuffer(this->cyclicBufferSize);
    checkNew(this->buff);

    this->t = 0;
    this->f = 0;
}

void monitorServer::initializeServer() {
    if (gethostname(this->machineName, sizeof(this->machineName)) == -1) {
        perror("gethostname");
        exit(1);
    }
    if ((this->ip = gethostbyname(this->machineName)) == nullptr) {
        perror("gethostbyname");
        exit(1);
    }
    this->ip_addr = (struct in_addr**)this->ip->h_addr_list;
    strcpy(this->externalAddress, inet_ntoa(*(this->ip_addr[0])));
    cout << "Monitor machineName " << this->machineName << endl;
    cout << "Monitor externalAddress " << this->externalAddress << endl;
    int listen_sock;
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("travelMonitorClient error opening socket");
        exit(-1);
    }
    int optionE = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &optionE, sizeof(optionE)) < 0) {
        perror("Couldn't set Socket Options");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    // server.sin_addr.s_addr = htonl(INADDR_ANY);
    this->server.sin_addr.s_addr = inet_addr(this->externalAddress);
    server.sin_port = htons(this->port);

    this->serverptr = (struct sockaddr*)&(this->server);
    unsigned int serverlen = sizeof(this->server);

    if (bind(listen_sock, this->serverptr, serverlen) < 0) {
        perror(" bind ");
        exit(-1);
    }
    cout << "binded" << endl;
    if (listen(listen_sock, 1) < 0) {
        perror(" listen ");
        exit(-1);
    }
    cout << "listening" << endl;
    if ((this->sock = accept(listen_sock, serverptr, &serverlen)) < 0) {
        perror("accept");
        exit(-1);
    }
    cout << "accepted" << endl;
}

void* threadFunc(void* args) {
    monitor.openPathsByThreads();
}

void monitorServer::openThreads() {
    pthread_t threads[this->numThreads];
    pthread_mutex_init(&(this->mutex), NULL);

    for (int i = 0; i < this->numThreads; i++) {
        cout << "creating thread, " << i << endl;
        if (pthread_create(&threads[i], nullptr, threadFunc, nullptr) != 0) {
            perror("create thread");
            exit(-1);
        }
    }

    while (1) {
        // this->buff->put();
    }

    for (int i = 0; i < this->numThreads; i++) {
        cout << "pthread_join, " << i << endl;
        if (pthread_join(threads[i], nullptr) != 0) {
            perror("join thread");
            exit(-1);
        }
    }
    pthread_mutex_destroy(&(this->mutex));
}


void monitorServer::openPathsByThreads() {
    while (1) {
        string FILE = this->buff->take();
        if (FILE.compare("..") == 0 || FILE.compare(".") == 0)
            continue;
        if (this->addNewFile(FILE)) {
            this->addFromFile(FILE);
        }
    }
}

void monitorServer::receiveId() {
    int send;
    if (read(this->sock, &send, sizeof(int)) == -1)
        cout << "Error in writting sizeOfStr with errno=" << errno << endl;
    this->id = send;
    cout << "Monitor " << this->id << endl;
    cout << receiveStr() << endl;
}

void monitorServer::addFromFile(string filepath)
{
    ifstream file(filepath);
    string line;
    string* words = new string[8];
    int length = 0;
    while (getline(file, line))
    {
        // cout << line << endl;
        splitString(&words, line, &length);
        pthread_mutex_lock(&(this->mutex));
        addRecord(length, words, line);
        pthread_mutex_unlock(&(this->mutex));
        length = 0;
    }
    delete[] words;
    file.close();
}

void monitorServer::addRecord(int length, string* words, string line)
{

    citizenRecord* citizen;
    if (this->checkSyntaxRecord("ERROR IN RECORD ", length, words, line)) // the record had syntax errors
        return;

    addNewVirus(words[5]);
    stringList* virus = this->viruses->search(words[5]);
    addNewCountry(words[3]);
    stringList* country = this->countries->search(words[3]);

    char status = 'n';
    string date = "";
    if (words[6].compare("YES") == 0)
    {
        status = 'y';
        date = words[7];
    }

    citizen = new citizenRecord(stoi(words[0]), words[1], words[2], country, stoi(words[4]), virus, status, date);
    checkNew(citizen);

    string result = "";
    citizenRecord* alreadyInTree = nullptr;

    tree = this->tree->insert(tree, citizen, &alreadyInTree, &result, false); // insert in tree

    if (result.compare("NEW CITIZEN") == 0)
    {
        if (status == 'y')
        {
            this->blooms->getBloom(virus)->add(citizen->getID());
            this->skipLists->getVaccinated(virus)->add(citizen->getID(), citizen);
        }
        else if (status == 'n')
            this->skipLists->getNotVaccinated(virus)->add(citizen->getID(), citizen);
    }
    else if (result.compare("VIRUS ADDED TO CITIZEN") == 0)
    {
        if (status == 'y')
        {
            this->blooms->getBloom(virus)->add(alreadyInTree->getID());
            this->skipLists->getVaccinated(virus)->add(alreadyInTree->getID(), alreadyInTree);
        }
        else if (status == 'n')
            this->skipLists->getNotVaccinated(virus)->add(alreadyInTree->getID(), alreadyInTree);
    }
    else if (result.compare("WRONG CREDENTIALS") == 0)
    {
        if (length == 8)
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
        else
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << endl;

        cout << "ERROR= " << result << endl;
    }
    else if (result.compare("VIRUS DUPLICATE") == 0)
    {
        if (length == 8)
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
        else
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << endl;

        cout << "ERROR= " << result << endl;
    }
}

int monitorServer::checkSyntaxRecord(string errorMessage, int length, string* words, string input)
{
    if (length < 7 || length > 8) // record must be only 7 or 8 words
    {
        errorMessage.pop_back();
        cout << errorMessage << " " << input << endl;
        cout << "ERROR= ARGUMENT LENGTH ERROR" << endl;
        return 1;
    }
    if (words[0].find_first_not_of("0123456789") != -1)
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= ID ERROR" << endl;
        return 1;
    }
    if (stoi(words[0]) > 9999 && stoi(words[0]) <= 0) // id is 4 digits
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= ID ERROR" << endl;
        return 1;
    }
    if (words[4].find_first_not_of("0123456789") != -1)
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= AGE ERROR" << endl;
        return 1;
    }
    if (stoi(words[4]) > 120 && stoi(words[4]) <= 0) // age is from 1 to 120
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= AGE ERROR" << endl;
        return 1;
    }
    if (words[6].compare("NO") != 0 && words[6].compare("YES") != 0) // the 6 word must only be NO or YES
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= YES/NO ERROR" << endl;
        return 1;
    }
    if (length == 8 && (words[6].compare("NO") == 0)) // when we have 8 words the word must be YES
    {
        cout << errorMessage << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
        cout << "ERROR= NO WITH DATE ERROR" << endl;
        return 1;
    }
    if (length == 7 && (words[6].compare("YES") == 0)) // when we have 7 words the word must be NO
    {
        cout << errorMessage << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << endl;
        cout << "ERROR= YES WITHOUT DATE ERROR" << endl;
        return 1;
    }
    if (length == 8)
    {
        date checker(words[7]);
        if (!checker.isValid())
        {
            cout << errorMessage << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
            cout << "ERROR= DATE FORMAT" << endl;
            return 1;
        }
    }
    return 0;
}

// void monitorServer::sendBlooms() {

//     stringList* temp = this->viruses;
//     while (temp != nullptr) {
//         sendStr(temp->getString());
//         temp = temp->getNext();
//     }
//     int end = -1;
//     if (write(writeFD, &end, sizeof(int)) == -1)
//         cout << "Error in writting end with errno=" << errno << endl;

//     temp = this->viruses;
//     while (temp != nullptr) {
//         bloomFilter* bloomV = this->blooms->getBloom(temp);
//         sendStr(temp->getString());
//         int pos = 0;
//         char* bloomArray = bloomV->getArray();

//         for (int i = 0;i <= this->bloomSize / this->socketBufferSize;i++) {
//             if (write(writeFD, &bloomArray[pos], this->socketBufferSize) == -1)
//                 cout << "Error in writting i with errno=" << errno << endl;
//             pos += this->socketBufferSize;
//         }

//         temp = temp->getNext();
//     }

//     sendStr("END BLOOMS");
//     receiveDone();
// }

// void monitorServer::receiveDone()
// {
//     receiveStr();
// }

void monitorServer::addNewVirus(string virusName)
{
    if (this->viruses->search(virusName) == nullptr) // if we dont have that virus add it to the list of viruses
    {                                         // and make the bloom filter and the skiplist for that virus
        this->viruses = this->viruses->add(virusName);
        this->blooms = this->blooms->add(this->viruses);
        this->skipLists = this->skipLists->add(this->viruses);
    }
}

void monitorServer::addNewCountry(string countryName)
{
    if (this->countries->search(countryName) == nullptr) // if we dont have that country add it to the list of Countries
    {
        this->countries = this->countries->add(countryName);
    }
}

int monitorServer::addNewFile(string file)
{
    if (this->filesReaded->search(file) == nullptr) // if we dont have that country add it to the list of Countries
    {
        pthread_mutex_lock(&(this->mutex));
        this->filesReaded = this->filesReaded->add(file);
        pthread_mutex_unlock(&(this->mutex));
        return 1;
    }
    return 0;
}

void monitorServer::sendStr(string str) {
    char* to_tranfer = &str[0];
    int sizeOfStr = strlen(to_tranfer);

    if (write(this->sock, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Error in writting sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr > this->socketBufferSize) {
        int pos = 0;
        for (int i = 0;i <= strlen(to_tranfer) / this->socketBufferSize;i++) {
            if (write(this->sock, &to_tranfer[pos], this->socketBufferSize) == -1)
                if (errno != 4)
                    cout << "Error in writting to_tranfer with errno=" << errno << endl;
            pos += this->socketBufferSize;
        }
    }
    else
        if (write(this->sock, &to_tranfer[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;

}

string monitorServer::receiveStr() {
    int sizeOfStr;
    if (read(this->sock, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "monitorServer " << this->id << " error in reading sizeOfStr with errno=" << errno << endl;

    string str = "";
    if (sizeOfStr > this->socketBufferSize) {
        for (int i = 0;i <= sizeOfStr / this->socketBufferSize;i++) {
            char buff[this->socketBufferSize + 1];
            if (read(this->sock, &buff[0], this->socketBufferSize) == -1)
                if (errno != 4)
                    cout << "monitorServer " << this->id << " error in reading buff with errno=" << errno << endl;
            buff[this->socketBufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(this->sock, &buff[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "monitorServer " << this->id << " error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

string monitorServer::receiveManyStr(int* end) {
    int sizeOfStr;
    if (read(this->sock, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "monitorServer " << this->id << " error in reading sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr == -1) {
        *end = -1;
        return "";
    }

    string str = "";
    if (sizeOfStr > this->socketBufferSize) {
        for (int i = 0;i <= sizeOfStr / this->socketBufferSize;i++) {
            char buff[this->socketBufferSize + 1];
            if (read(this->sock, &buff[0], this->socketBufferSize) == -1)
                if (errno != 4)
                    cout << "monitorServer " << this->id << " error in reading buff with errno=" << errno << endl;
            buff[this->socketBufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(this->sock, &buff[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "monitorServer " << this->id << " error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        // cout << "test " << buff << endl;
        str.append(buff);
    }
    return str;
}

void monitorServer::printAllCountries() {
    cout << "monitorServer " << this->id << " has the countries:";
    this->countries->print();
}

void monitorServer::printAllViruses() {
    cout << "monitorServer " << this->id << " has the viruses:";
    this->viruses->print();
}