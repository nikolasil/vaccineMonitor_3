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
#include <signal.h>

#include "monitor.h"
#include "DataStructures/stringList/stringList.h"
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/skipList/skipList.h"
#include "DataStructures/binaryAvlTree/tree.h"
#include "citizenRecords/citizen.h"
#include "util.h"

using namespace std;

Monitor monitor = Monitor();

void signal_handler_SIGINT_SIGQUIT(int signo) {
    // cout << "signal_handler_SIGINT_SIGQUIT" << endl;
    monitor.makeLogFile();
    monitor.suicide();
}
void signal_handler_SIGUSR1(int signo) {
    // cout << "signal_handler_SIGUSR1" << endl;
    monitor.readFilesAndCreateStructures();
    monitor.sendBlooms();
}

void Monitor::suicide() {
    if (this->tree != NULL)
        delete this->tree;
    if (this->skipLists != NULL)
        delete this->skipLists;
    if (this->blooms != NULL)
        delete this->blooms;
    if (this->viruses != NULL)
        delete this->viruses;
    if (this->countries != NULL)
        delete this->countries;
    if (this->filesReaded != NULL)
        delete this->filesReaded;
    if (this->command != NULL)
        delete[] this->command;
    close(readFD);
    close(writeFD);
    cout << "Monitor " << getpid() << " Terminated" << endl;
    exit(1);
}

void Monitor::waitForCommands() {
    while (1)
    {
        // cout << "Waiting for commands " << this->id << endl;
        string input = receiveStr();
        int length;
        this->command = readString(input, &length);
        if (length > 0)
        {
            if (command[0].compare("/travelRequest") == 0)
                this->travelRequest(command, length);

            if (command[0].compare("/searchVaccinationStatus") == 0)
                this->searchVaccinationStatus(command, length);

            else
                cout << "Invalid command!" << endl;
        }
        if (command != NULL)
            delete[] command;
    }
}

void Monitor::travelRequest(string* arguments, int length) {
    id = stoi(arguments[1]);
    date checkDate = date((arguments[2]));
    string countryFrom = arguments[3];
    string countryTo = arguments[4];
    string virusName = arguments[5];

    stringList* v = this->viruses->search(virusName);
    if (v != NULL)
    {
        skipList* l;
        l = skipLists->getVaccinated(v);
        skipListNode* n = l->search(id, 't');

        if (n == NULL) {
            sendStr("NO");
            f++;
        }
        else {
            sendStr("YES " + n->getCitizen()->getStatus()->getVirusDate(v).getConcatenate());
            t++;
        }
    }
    else {
        sendStr("NO");
        f++;
    }
}

void Monitor::searchVaccinationStatus(string* arguments, int length) {
    id = stoi(arguments[1]);
    treeNode* citizen = this->tree->search(this->tree, id);
    if (citizen != NULL) {
        int end = 1;
        if (write(writeFD, &end, sizeof(int)) == -1)
            if (errno != 4)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;
        string credentials = arguments[1] + " " + citizen->getCitizen()->getFirstName();
        credentials.append(" " + citizen->getCitizen()->getLastName());
        credentials.append(" " + citizen->getCitizen()->getCountry()->getString());
        sendStr(credentials);

        string age = "AGE ";
        age.append(to_string(citizen->getCitizen()->getAge()));
        sendStr(age);

        listStatus* status = citizen->getCitizen()->getStatus();
        while (status != NULL) {
            // cout << "1" << endl;
            string virusStatus = "";
            virusStatus.append(status->getVirusName()->getString());
            if (status->getVirusStatus() == 'y') {
                virusStatus.append(" VACCINATED ON ");
                virusStatus.append(status->getDateVaccinated().getConcatenate());
            }
            else {
                virusStatus.append(" NOT YET VACCINATED");
            }
            sendStr(virusStatus);
            status = status->getNext();
        }
        end = -1;
        if (write(writeFD, &end, sizeof(int)) == -1)
            if (errno != 4)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;
    }
    else {
        int end = -1;
        if (write(writeFD, &end, sizeof(int)) == -1)
            if (errno != 4)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;
    }
}

void Monitor::makeLogFile() {
    int pid = getpid();
    ofstream logfile("logfiles/log_file." + to_string(pid));
    stringList* temp = this->countries;
    while (temp != NULL) {
        logfile << temp->getString() << endl;
        temp = temp->getNext();
    }
    logfile << "TOTAL TRAVEL REQUESTS " << this->t + this->f << endl;
    logfile << "ACCEPTED " << this->t << endl;
    logfile << "REJECTED " << this->f << endl;
    logfile.close();
}

Monitor::Monitor() {

    handlerSIGINT_SIGQUIT.sa_handler = signal_handler_SIGINT_SIGQUIT;
    sigemptyset(&(handlerSIGINT_SIGQUIT.sa_mask));
    handlerSIGINT_SIGQUIT.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &handlerSIGINT_SIGQUIT, NULL);
    sigaction(SIGINT, &handlerSIGINT_SIGQUIT, NULL);

    handlerSIGUSR1.sa_handler = signal_handler_SIGUSR1;
    sigemptyset(&(handlerSIGUSR1.sa_mask));
    handlerSIGUSR1.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &handlerSIGUSR1, NULL);
}

void Monitor::start(string r, string w) {
    this->readFifo = r;
    this->writeFifo = w;
    readFD = open(this->readFifo.c_str(), O_RDONLY);
    writeFD = open(this->writeFifo.c_str(), O_WRONLY);

    this->tree = NULL;
    this->countries = new stringList();
    checkNew(this->countries);

    this->viruses = new stringList();
    checkNew(this->viruses);

    this->filesReaded = new stringList();
    checkNew(this->filesReaded);

    this->skipLists = new skipList_List();
    checkNew(this->skipLists);

    this->t = 0;
    this->f = 0;
}

Monitor::~Monitor() {
    close(readFD);
    close(writeFD);
}

void Monitor::receiveCredentials() {
    // cout << "monitor cred " << readFD << endl;
    if (read(readFD, &this->id, sizeof(int)) == -1)
        cout << "Monitor error in reading id with errno=" << errno << endl;
    if (read(readFD, &this->bufferSize, sizeof(int)) == -1)
        cout << "Monitor " << this->id << " error in reading bufferSize with errno=" << errno << endl;
    if (read(readFD, &this->bloomSize, sizeof(int)) == -1)
        cout << "Monitor " << this->id << " error in reading bloomSize with errno=" << errno << endl;
    this->blooms = new bloomFilterList(this->bloomSize);
    checkNew(this->blooms);
    this->generalDirectory = receiveStr();
    cout << "Monitor got" << this->id << ", bufferSize=" << this->bufferSize << ", bloomSize=" << this->bloomSize << ", generalDirectory=" << this->generalDirectory << endl;

}

void Monitor::receiveCountries() {
    int end = 0;
    while (end != -1) {
        string country = receiveManyStr(&end);
        if (country != "") {
            this->addNewCountry(country);
            cout << "Monitor " << this->id << " got country=" << country << endl;
        }
    }
}

void Monitor::readFilesAndCreateStructures() {
    stringList* country = this->countries;

    while (country != NULL) {
        DIR* input;
        struct dirent* dir;
        string in = this->generalDirectory;
        in.append(country->getString());
        char* in2 = &in[0];
        input = opendir(in2);

        // cout << "Monitor " << this->id << " " << in2 << endl;
        if (input)
        {
            while ((dir = readdir(input)) != NULL)
            {
                string FILE = dir->d_name;
                if (FILE.compare("..") == 0 || FILE.compare(".") == 0)
                    continue;
                // cout << "Monitor " << this->id << " " << FILE << endl;
                string fullpath = in;
                fullpath.append("/");
                fullpath.append(FILE);
                if (this->addNewFile(fullpath)) {
                    cout << "File: " << fullpath << endl;
                    this->addFromFile(fullpath);
                }
            }
        }
        country = country->getNext();
    }
}

void Monitor::addFromFile(string filepath)
{
    // cout << filepath << endl;
    ifstream file(filepath);
    string line;
    string* words = new string[8];
    int length = 0;
    while (getline(file, line))
    {
        // cout << line << endl;
        splitString(&words, line, &length);
        addRecord(length, words, line);
        length = 0;
    }
    delete[] words;
    file.close();
}

void Monitor::addRecord(int length, string* words, string line)
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
    citizenRecord* alreadyInTree = NULL;

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

int Monitor::checkSyntaxRecord(string errorMessage, int length, string* words, string input)
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

void Monitor::sendBlooms() {

    stringList* temp = this->viruses;
    while (temp != NULL) {
        sendStr(temp->getString());
        temp = temp->getNext();
    }
    int end = -1;
    if (write(writeFD, &end, sizeof(int)) == -1)
        cout << "Error in writting end with errno=" << errno << endl;

    temp = this->viruses;
    while (temp != NULL) {
        bloomFilter* bloomV = this->blooms->getBloom(temp);
        sendStr(temp->getString());
        int pos = 0;
        char* bloomArray = bloomV->getArray();

        for (int i = 0;i <= this->bloomSize / this->bufferSize;i++) {
            if (write(writeFD, &bloomArray[pos], bufferSize) == -1)
                cout << "Error in writting i with errno=" << errno << endl;
            pos += this->bufferSize;
        }

        temp = temp->getNext();
    }

    sendStr("END BLOOMS");
    receiveDone();
}

void Monitor::receiveDone()
{
    receiveStr();
}

void Monitor::addNewVirus(string virusName)
{
    if (this->viruses->search(virusName) == NULL) // if we dont have that virus add it to the list of viruses
    {                                         // and make the bloom filter and the skiplist for that virus
        this->viruses = this->viruses->add(virusName);
        this->blooms = this->blooms->add(this->viruses);
        this->skipLists = this->skipLists->add(this->viruses);
    }
}

void Monitor::addNewCountry(string countryName)
{
    if (this->countries->search(countryName) == NULL) // if we dont have that country add it to the list of Countries
    {
        this->countries = this->countries->add(countryName);
    }
}

int Monitor::addNewFile(string file)
{
    if (this->filesReaded->search(file) == NULL) // if we dont have that country add it to the list of Countries
    {
        this->filesReaded = this->filesReaded->add(file);
        return 1;
    }
    return 0;
}

void Monitor::sendStr(string str) {
    char* to_tranfer = &str[0];
    int sizeOfStr = strlen(to_tranfer);

    if (write(writeFD, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Error in writting sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr > this->bufferSize) {
        int pos = 0;
        for (int i = 0;i <= strlen(to_tranfer) / this->bufferSize;i++) {
            if (write(writeFD, &to_tranfer[pos], this->bufferSize) == -1)
                if (errno != 4)
                    cout << "Error in writting to_tranfer with errno=" << errno << endl;
            pos += this->bufferSize;
        }
    }
    else
        if (write(writeFD, &to_tranfer[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;

}

string Monitor::receiveStr() {
    int sizeOfStr;
    if (read(readFD, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Monitor " << this->id << " error in reading sizeOfStr with errno=" << errno << endl;

    string str = "";
    if (sizeOfStr > this->bufferSize) {
        for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
            char buff[this->bufferSize + 1];
            if (read(readFD, &buff[0], this->bufferSize) == -1)
                if (errno != 4)
                    cout << "Monitor " << this->id << " error in reading buff with errno=" << errno << endl;
            buff[this->bufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(readFD, &buff[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Monitor " << this->id << " error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

string Monitor::receiveManyStr(int* end) {
    int sizeOfStr;
    if (read(readFD, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Monitor " << this->id << " error in reading sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr == -1) {
        *end = -1;
        return "";
    }

    string str = "";
    if (sizeOfStr > this->bufferSize) {
        for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
            char buff[this->bufferSize + 1];
            if (read(readFD, &buff[0], this->bufferSize) == -1)
                if (errno != 4)
                    cout << "Monitor " << this->id << " error in reading buff with errno=" << errno << endl;
            buff[this->bufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(readFD, &buff[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Monitor " << this->id << " error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        // cout << "test " << buff << endl;
        str.append(buff);
    }
    return str;
}

void Monitor::printAllCountries() {
    cout << "Monitor " << this->id << " has the countries:";
    this->countries->print();
}

void Monitor::printAllViruses() {
    cout << "Monitor " << this->id << " has the viruses:";
    this->viruses->print();
}