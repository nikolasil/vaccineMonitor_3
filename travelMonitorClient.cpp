#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <typeinfo>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "travelMonitorClient.h" 
#include "util.h" 
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/date/date.h"

#define portNumber 7777

using namespace std;

travelMonitorClient mainMonitor = travelMonitorClient();

void travelMonitorClient::suicide() {
    if (this->monitors != nullptr)
        delete this->monitors;
    if (this->countryToMonitor != nullptr)
        delete this->countryToMonitor;
    if (this->requests != nullptr)
        delete this->requests;
    if (this->blooms != nullptr)
        delete this->blooms;
    if (this->viruses != nullptr)
        delete this->viruses;
    if (this->countries != nullptr)
        delete this->countries;
    if (this->command != nullptr)
        delete[] this->command;

    cout << "Travel Monitor Terminated" << endl;
    exit(0);
}

void travelMonitorClient::killAllMonitors() {
    for (int i = 0;i < numMonitors;i++) {
        cout << "Send SIGKILL to monitor " << this->monitors->getPID(i) << endl;
        // sendSIGKILL(i);
    }
}

travelMonitorClient::travelMonitorClient() {}

void travelMonitorClient::start(int m, int b, int c, int s, string dir, int t) {
    this->numMonitors = m;
    this->socketBufferSize = b;
    this->cyclicBufferSize = c;
    this->sizeOfBloom = s;
    this->input_dir = dir;
    this->numThreads = t;

    this->countryToMonitor = nullptr;
    this->monitors = nullptr;
    this->viruses = new stringList();
    checkNew(this->viruses);
    this->filePaths = new stringList();
    checkNew(this->filePaths);

    this->blooms = new bloomFilterList(this->sizeOfBloom);
    checkNew(this->blooms);
}

void travelMonitorClient::roundRobinCountriesandPutToList() {
    int monitor = 0;
    int count;

    struct dirent** coutriesDir;
    count = scandir(this->input_dir.c_str(), &coutriesDir, nullptr, alphasort);
    if (count < 0)
        perror("error in scandir");
    else {
        for (int i = 0;i < count;i++) {
            string country = coutriesDir[i]->d_name;
            if (country.compare("..") == 0 || country.compare(".") == 0) {
                free(coutriesDir[i]);
                continue;
            }

            this->addCountryToMonitor(country, monitor);

            monitor++;
            if (monitor >= this->numMonitors)
                monitor = 0;
            free(coutriesDir[i]);
        }
        free(coutriesDir);
    }
}

void travelMonitorClient::createServers() {
    for (int i = 0;i < numMonitors;i++)
        createServer(i);
}
void travelMonitorClient::createServer(int i) {
    pid_t c_pid = fork();
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (c_pid > 0) {
        cout << "Monitor " << i << " Created with pid " << c_pid << endl;
        this->addMonitor(c_pid, i);
        // sleep(2);
    }
    else {
        int port = portNumber + i;
        string files = "";
        files.append("-p " + to_string(port));
        files.append(" -t " + to_string(this->numThreads));
        files.append(" -b " + to_string(this->socketBufferSize));
        files.append(" -c " + to_string(this->cyclicBufferSize));
        files.append(" -s " + to_string(this->sizeOfBloom));
        monitorCountryPairList* temp = this->countryToMonitor;
        while (temp != nullptr) {
            if (i == temp->getMonitor())
                files.append(" " + this->input_dir + temp->getCountry());
            temp = temp->getNext();
        }
        int length;
        string* argArray = readString(files, &length);
        char* args[length + 1];
        args[length] = nullptr;
        for (int i = 0;i < length;i++) {
            char* word = new char[argArray[i].length() + 1];
            word[argArray[i].length()] = '\0';
            for (int j = 0;j < argArray[i].length();j++)
                word[j] = (argArray[i].c_str())[j];
            args[i] = word;
        }
        char* server = "./monitorServer";
        execvp(server, args);
    }
}

void travelMonitorClient::openSockets() {
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
    cout << "machineName " << this->machineName << endl;
    cout << "externalAddress " << this->externalAddress << endl;

    for (int i = 0;i < numMonitors;i++) {
        int port = portNumber + i;
        int sock;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("travelMonitorClient error opening socket");
            exit(-1);
        }
        int optionE = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optionE, sizeof(optionE)) < 0) {
            perror("optionE setsockopt error");
            exit(-1);
        }
        this->client.sin_family = AF_INET;
        // this->client.sin_addr.s_addr = htonl(INADDR_ANY);
        this->client.sin_addr.s_addr = inet_addr(this->externalAddress);
        this->client.sin_port = htons(port); /* The given port */
        this->clientptr = (struct sockaddr*)&(this->client);
        unsigned int clientlen = sizeof(this->client);
        int connectStatus;
        do
            connectStatus = connect(sock, clientptr, clientlen);
        while (connectStatus < 0);
        cout << "connected to monitor " << i << endl;
        cout << "fd " << sock << endl;
        this->addFdToMonitor(i, sock);
    }
}

void travelMonitorClient::sendIds() {
    for (int i = 0;i < numMonitors;i++) {
        if (write(this->monitors->getSocketFD(i), &i, sizeof(int)) == -1)
            cout << "Error in writting sizeOfStr with errno=" << errno << endl;
    }
}

void travelMonitorClient::receiveBlooms() {
    fd_set fileDecriptorSet;

    int totalRead = 0;
    int flag = 0;
    while (!flag) {
        FD_ZERO(&fileDecriptorSet);
        for (int i = 0;i < this->numMonitors;i++) {
            FD_SET(this->monitors->getSocketFD(i), &fileDecriptorSet);
        }
        select(this->monitors->getSocketFD(this->numMonitors - 1) + 1, &fileDecriptorSet, nullptr, nullptr, nullptr);

        for (int i = 0;i < this->numMonitors;i++) {
            if (FD_ISSET(this->monitors->getSocketFD(i), &fileDecriptorSet)) {
                cout << "Monitor" << i << " is ready!" << endl;

                receiveBlooms(i);

                totalRead++;
            }
            if (totalRead == this->numMonitors) {
                flag = 1;
                break;
            }
        }
    }

    sendDone();
}

void travelMonitorClient::receiveBlooms(int i) {
    cout << "travelMonitorClient receiveBlooms" << endl;
    int end = 0;
    while (1) {
        string virus = receiveManyStr(i, &end);
        if (end == -1 || virus == "")
            break;
        addNewVirus(virus);
        // cout << "Got virus=" << virus << " from Monitor " << i << endl;
    }
    cout << "Updating blooms from Monitor " << i << endl;
    while (1) {

        string virus = receiveStr(i);

        if (virus.compare("END BLOOMS") == 0) {
            cout << "Done updating blooms" << endl;
            break;
        }

        int pos = 0;
        int fd = this->monitors->getSocketFD(i);
        char* bloomArray = this->blooms->getBloom(this->viruses->search(virus))->getArray();
        for (int i = 0;i <= this->sizeOfBloom / this->socketBufferSize;i++) {
            char buff[this->socketBufferSize];
            if (recv(fd, &buff, this->socketBufferSize, MSG_WAITALL) == -1)
                cout << "Error in reading bit with errno=" << errno << endl;

            for (int i = 0; i < this->socketBufferSize;i++)
                bloomArray[pos + i] = bloomArray[pos + i] | buff[i];

            pos += this->socketBufferSize;
        }
        // cout << virus << " ";
        // this->blooms->getBloom(this->viruses->search(virus))->print();
    }
}

void travelMonitorClient::sendDone() {
    for (int i = 0;i < numMonitors;i++)
        sendStr(i, "DONE");
}

void travelMonitorClient::startMenu() {
    while (1)
    {
        string input = getInput("Enter command: ");
        int length;
        this->command = readString(input, &length);
        if (length > 0)
        {
            if (command[0].compare("/travelRequest") == 0)
                this->travelRequest(command, length);

            else if (command[0].compare("/travelStats") == 0)
                this->travelStats(command, length);

            else if (command[0].compare("/addVaccinationRecords") == 0)
                this->addVaccinationRecords(command, length);

            else if (command[0].compare("/searchVaccinationStatus") == 0)
                this->searchVaccinationStatus(command, length);

            else if (command[0].compare("/exit") == 0) {
                for (int i = 0;i < this->numMonitors;i++) {
                    sendStr(i, "/exit");
                    string res = receiveStr(i);
                    cout << "Monitor " << i << res << endl;
                }
                this->suicide();
            }
            else
                cout << "Invalid command!" << endl;
        }
        delete[] command;
    }
}

void travelMonitorClient::travelRequest(string* arguments, int length) {
    cout << endl;
    cout << "- Selected: /travelRequest" << endl;
    if (length == 6)
    {
        int id;
        if (arguments[1].find_first_not_of("0123456789") != -1)
        {
            cout << "ERROR citizenID must be given as first argument && it must be a 4 digit integer" << endl;
            return;
        }
        else if (stoi(arguments[1]) > 9999 && stoi(arguments[1]) <= 0)
        {
            cout << "ERROR citizenID must be given as first argument && it must be a 4 digit integer" << endl;
            return;
        }
        id = stoi(arguments[1]);
        date checkDate = date((arguments[2]));
        if (!checkDate.isValid()) {
            cout << "ERROR date must be given as second argument && it must be in in correct format (dd-mm-yyyy)" << endl;
            return;
        }
        string countryFrom = arguments[3];
        string countryTo = arguments[4];
        string virusName = arguments[5];

        cout << "- citizenID: " << id << endl;
        cout << "- date: ";
        checkDate.print();
        cout << endl;
        cout << "- countryFrom: " << countryFrom << endl;
        cout << "- countryTo: " << countryTo << endl;
        cout << "- virusName: " << virusName << endl;
        cout << endl;

        int res;
        stringList* v = this->viruses->search(virusName);
        if (v != nullptr)
            res = this->blooms->getBloom(v)->check(id);
        else
        {
            cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
            this->addRequest(countryFrom, virusName, checkDate, false);
            return;
        }

        if (res)
        {
            string s = "/travelRequest ";
            s.append(to_string(id) + " ");
            s.append(checkDate.getConcatenate() + " ");
            s.append(countryFrom + " ");
            s.append(countryTo + " ");
            s.append(virusName);
            if (countryToMonitor->search(countryFrom) != nullptr) {
                sendStr(countryToMonitor->search(countryFrom)->getMonitor(), s);
                string res = receiveStr(countryToMonitor->search(countryFrom)->getMonitor());
                // cout << res << endl;
                if (res == "NO") {
                    cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
                    this->addRequest(countryFrom, virusName, checkDate, false);
                    return;
                }
                int l;
                string* split = readString(res, &length);
                date vacDate = date((split[1]));

                int total1 = 0;
                int year1 = stoi(vacDate.getYear()) * 10000;
                int month1 = stoi(vacDate.getMonth()) * 100;
                int day1 = stoi(vacDate.getDay());
                total1 = total1 + year1 + month1 + day1;

                int total2 = 0;
                int year2 = stoi(checkDate.getYear()) * 10000;
                int month2 = stoi(checkDate.getMonth()) * 100;
                int day2 = stoi(checkDate.getDay());
                total2 = total2 + year2 + month2 + day2;

                if (total2 - total1 > 600) {
                    cout << "REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE" << endl;
                    this->addRequest(countryFrom, virusName, checkDate, false);
                    return;
                }
                else if (total2 - total1 >= 0) {
                    cout << "REQUEST ACCEPTED – HAPPY TRAVELS" << endl;
                    this->addRequest(countryFrom, virusName, checkDate, true);
                    return;
                }
                else {
                    cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
                    this->addRequest(countryFrom, virusName, checkDate, false);
                    return;
                }
            }
            else {
                cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
                this->addRequest(countryFrom, virusName, checkDate, false);
                return;
            }
        }
        else {
            cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
            this->addRequest(countryFrom, virusName, checkDate, false);
            return;
        }
    }
    else {
        cout << "ERROR 5 Arguments must be given" << endl;
        cout << "ERROR Total arguments given was: " << length - 1 << endl;
    }
}

void travelMonitorClient::travelStats(string* arguments, int length) {
    cout << endl;
    cout << "- Selected: /travelStats" << endl;

    if (!(length == 4 || length == 5)) {
        cout << "ERROR 3-4 Arguments must be given" << endl;
        cout << "ERROR Total arguments given was: " << length - 1 << endl;
        return;
    }

    int t = 0;
    int f = 0;
    string virusName = arguments[1];
    date checkDate1 = date((arguments[2]));
    if (!checkDate1.isValid()) {
        cout << "ERROR date1 must be given as second argument && it must be in in correct format (dd-mm-yyyy)" << endl;
        return;
    }
    date checkDate2 = date((arguments[3]));
    if (!checkDate2.isValid()) {
        cout << "ERROR date2 must be given as third argument && it must be in in correct format (dd-mm-yyyy)" << endl;
        return;
    }
    if (checkDate1.compare(checkDate2) > 0) {
        cout << "ERROR date1 must be smaller than date2" << endl;
        return;
    }
    cout << "- virusName: " << virusName << endl;
    cout << "- date1: ";
    checkDate1.print();
    cout << endl;
    cout << "- date2: ";
    checkDate2.print();
    cout << endl;

    if (length == 4) {
        cout << endl;
        statsList* temp = this->requests;
        while (temp != nullptr) {
            if (checkDate1.compare(temp->getDate()) <= 0 && checkDate2.compare(temp->getDate()) >= 0) {
                if (temp->getVirusName() == virusName) {
                    temp->getStat() ? t++ : f++;
                }
            }
            temp = temp->getNext();
        }
        cout << "TOTAL REQUESTS " << t + f << endl;
        cout << "ACCEPTED " << t << endl;
        cout << "REJECTED " << f << endl;
        cout << endl;
    }
    else if (length == 5) {
        string country = arguments[4];
        cout << "- country: " << country << endl;
        cout << endl;
        statsList* temp = this->requests;
        while (temp != nullptr) {
            if (checkDate1.compare(temp->getDate()) <= 0 && checkDate2.compare(temp->getDate()) >= 0) {
                if (temp->getVirusName() == virusName && temp->getCountry() == country) {
                    temp->getStat() ? t++ : f++;
                }
            }
            temp = temp->getNext();
        }
        cout << "TOTAL REQUESTS " << t + f << endl;
        cout << "ACCEPTED " << t << endl;
        cout << "REJECTED " << f << endl;
        cout << endl;
    }
    else {
        cout << "ERROR 3-4 Arguments must be given" << endl;
        cout << "ERROR Total arguments given was: " << length - 1 << endl;
    }
}

void travelMonitorClient::addVaccinationRecords(string* arguments, int length) {
    cout << endl;
    cout << "- Selected: /addVaccinationRecords" << endl;

    if (length == 2) {
        string country = arguments[1];
        cout << "- country: " << country << endl;
        if (this->countryToMonitor->search(country) != nullptr) {
            sendStr(countryToMonitor->search(country)->getMonitor(), "/addVaccinationRecords " + this->input_dir + country);
            mainMonitor.receiveBlooms(this->countryToMonitor->search(country)->getMonitor());
            mainMonitor.sendStr(this->countryToMonitor->search(country)->getMonitor(), "DONE");
        }
        else {
            cout << "ERROR country= " << country << " dont exist" << endl;
        }
    }
    else {
        cout << "ERROR 1 Arguments must be given" << endl;
        cout << "ERROR Total arguments given was: " << length - 1 << endl;
    }
}

void travelMonitorClient::searchVaccinationStatus(string* arguments, int length) {
    cout << endl;
    cout << "- Selected: /searchVaccinationStatus" << endl;

    if (length == 2) {
        cout << "- citizenID: " << arguments[1] << endl;
        cout << endl;
        string s = "/searchVaccinationStatus";
        s.append(" " + arguments[1]);
        for (int i = 0;i < this->numMonitors;i++)
            sendStr(i, s);
        int monitor = -1;
        int buff;
        for (int i = 0;i < this->numMonitors;i++) {
            if (recv(this->monitors->getSocketFD(i), &buff, sizeof(int), MSG_WAITALL) == -1)
                if (errno != 4)
                    cout << "Error in reading sizeOfStr with errno=" << errno << endl;
            if (buff == 1) {
                monitor = i;
                cout << "Monitor " << monitor << " had that citizen" << endl;
                break;
            }
        }
        if (monitor == -1) {
            cout << "CITIZEN NOT FOUND" << endl;
            return;
        }
        int end = 1;
        while (end != -1) {
            cout << receiveManyStr(monitor, &end) << endl;
        }
        cout << endl;
    }
    else {
        cout << "ERROR 1 Arguments must be given" << endl;
        cout << "ERROR Total arguments given was: " << length - 1 << endl;
    }
}

void travelMonitorClient::sendStr(int monitor, string str) {
    cout << "sendStr " << str << endl;
    int fd = this->monitors->getSocketFD(monitor);
    char* to_tranfer = &str[0];
    int sizeOfStr = strlen(to_tranfer);

    if (write(fd, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Error in writting sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr > this->socketBufferSize) {
        int pos = 0;
        for (int i = 0;i <= strlen(to_tranfer) / this->socketBufferSize;i++) {
            if (write(fd, &to_tranfer[pos], this->socketBufferSize) == -1)
                if (errno != 4)
                    cout << "Error in writting to_tranfer with errno=" << errno << endl;

            pos += this->socketBufferSize;
        }
    }
    else
        if (write(fd, &to_tranfer[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;

}

string travelMonitorClient::receiveStr(int monitor) {
    int fd = this->monitors->getSocketFD(monitor);
    cout << "fd=" << fd << endl;
    int sizeOfStr;
    if (recv(fd, &sizeOfStr, sizeof(int), MSG_WAITALL) == -1)
        if (errno != 4)
            cout << "receiveStrError in reading sizeOfStr with errno=" << errno << endl;

    string str = "";
    if (sizeOfStr > this->socketBufferSize) {
        for (int i = 0;i <= sizeOfStr / this->socketBufferSize;i++) {
            char buff[this->socketBufferSize + 1];

            if (recv(fd, &buff[0], this->socketBufferSize, MSG_WAITALL) == -1)
                if (errno != 4)
                    cout << "receiveStrError in reading buff with errno=" << errno << " " << fd << endl;
            buff[this->socketBufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (recv(fd, &buff[0], sizeOfStr, MSG_WAITALL) == -1)
            if (errno != 4)
                // cout << "receiveStrError in reading buff with errno=" << errno << " " << fd << endl;
                perror("receiveStrError in reading buff with errno=");
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

string travelMonitorClient::receiveManyStr(int monitor, int* end) {
    int fd = this->monitors->getSocketFD(monitor);
    int sizeOfStr;
    if (recv(fd, &sizeOfStr, sizeof(int), MSG_WAITALL) == -1)
        if (errno != 4)
            cout << "receiveManyStrError in reading sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr == -1) {
        *end = -1;
        return "";
    }

    string str = "";
    if (sizeOfStr > this->socketBufferSize) {
        for (int i = 0;i <= sizeOfStr / this->socketBufferSize;i++) {
            char buff[this->socketBufferSize + 1];
            if (recv(fd, &buff[0], this->socketBufferSize, MSG_WAITALL) == -1)
                if (errno != 4)
                    cout << "receiveManyStrError in reading buff with errno=" << errno << endl;
            buff[this->socketBufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (recv(fd, &buff[0], sizeOfStr, MSG_WAITALL) == -1)
            if (errno != 4)
                cout << "receiveManyStrError in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

void travelMonitorClient::addCountryToMonitor(string c, int m) {
    if (this->countryToMonitor == nullptr)
        this->countryToMonitor = new monitorCountryPairList(c, m);
    else
        this->countryToMonitor = this->countryToMonitor->add(c, m);
}

void travelMonitorClient::addRequest(string c, string v, date dt, bool s) {
    if (this->requests == nullptr)
        this->requests = new statsList(c, v, dt, s);
    else
        this->requests = this->requests->add(c, v, dt, s);
}

void travelMonitorClient::addNewVirus(string virusName)
{
    if (this->viruses->search(virusName) == nullptr) // if we dont have that virus add it to the list of viruses
    {                                         // and make the bloom filter and the skiplist for that virus
        this->viruses = this->viruses->add(virusName);
        this->blooms = this->blooms->add(this->viruses);
    }
}

void travelMonitorClient::addMonitor(int pid, int id) {
    if (this->monitors == nullptr)
        this->monitors = new monitorList(pid, id);
    else
        this->monitors->add(pid, id);
}

void travelMonitorClient::addFdToMonitor(int m, int s) {
    this->monitors->addFD(m, s);
}

void travelMonitorClient::printAllViruses() {
    cout << "ALL THE viruses: ";
    this->viruses->print();
    cout << endl;
}

void travelMonitorClient::printCountryToMonitor() {
    this->countryToMonitor->print();
}