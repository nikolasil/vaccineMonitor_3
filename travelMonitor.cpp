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
#include <signal.h>

#include "travelMonitor.h" 
#include "util.h" 
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/date/date.h"

using namespace std;

travelMonitor mainMonitor = travelMonitor();

void signal_handler_SIGINT_SIGQUIT(int signo) {
    // cout << "TravelMonitor signal_handler_SIGINT_SIGQUIT" << endl;
    mainMonitor.killAllMonitors();
    mainMonitor.suicide();
}

void signal_handler_SIGCHLD(int signo) {
    // cout << "TravelMonitor signal_handler_SIGCHLD" << endl;
    pid_t pid;
    int status;
    cout << "Checking to see who terminated" << endl;
    while ((pid = waitpid(-1, &status, WNOHANG)) <= 0);
    if (status > 0) {
        cout << pid << " is terminated" << endl;
        int id = mainMonitor.getMonitors()->getID(pid);
        // cout << "The id was " << id << endl;
        mainMonitor.createMonitor(id);
        mainMonitor.openFifo(id);
        // cout << "line:43" << endl;
        mainMonitor.sendCredential(id);
        // cout << "line:45" << endl;
        monitorCountryPairList* temp = mainMonitor.getCountryToMonitor();
        temp->print();
        while (temp != NULL) {
            if (temp->getMonitor() == id) {
                // cout << "line:50 " << temp->getMonitor() << " " << temp->getCountry() << endl;
                mainMonitor.sendCountry(temp->getCountry(), id, false);
            }
            temp = temp->getNext();
        }
        // cout << "line:55" << endl;
        int fd = mainMonitor.getMonitors()->getWriteFifo(id);
        int end = -1;
        if (write(fd, &end, sizeof(int)) == -1)
            cout << "Error in writting end with errno=" << errno << endl;
        // cout << "line:62" << endl;
        mainMonitor.receiveBlooms(id);
        // cout << "line:64" << endl;
        mainMonitor.sendStr(id, "DONE");
        cout << "Done recreating Monitor" << endl;
    }
}


void travelMonitor::suicide() {
    if (this->monitors != NULL)
        delete this->monitors;
    if (this->countryToMonitor != NULL)
        delete this->countryToMonitor;
    if (this->requests != NULL)
        delete this->requests;
    if (this->blooms != NULL)
        delete this->blooms;
    if (this->viruses != NULL)
        delete this->viruses;
    if (this->countries != NULL)
        delete this->countries;
    if (this->command != NULL)
        delete[] this->command;

    cout << "Travel Monitor Terminated" << endl;
    kill(getpid(), SIGKILL);
}

void travelMonitor::sendSIGUSR1(int monitor) {
    kill(this->monitors->getPID(monitor), SIGUSR1);
}

void travelMonitor::sendSIGINT(int monitor) {
    kill(this->monitors->getPID(monitor), SIGINT);
}

void travelMonitor::sendSIGKILL(int monitor) {
    kill(this->monitors->getPID(monitor), SIGKILL);
}

void travelMonitor::sendSIGTERM(int monitor) {
    kill(this->monitors->getPID(monitor), SIGTERM);
}

void travelMonitor::killAllMonitors() {
    for (int i = 0;i < numMonitors;i++) {
        cout << "Send SIGKILL to monitor " << this->monitors->getPID(i) << endl;
        sendSIGKILL(i);
    }
}

travelMonitor::travelMonitor() {
    handlerSIGINT_SIGQUIT.sa_handler = signal_handler_SIGINT_SIGQUIT;
    sigemptyset(&(handlerSIGINT_SIGQUIT.sa_mask));
    handlerSIGINT_SIGQUIT.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &handlerSIGINT_SIGQUIT, NULL);
    sigaction(SIGINT, &handlerSIGINT_SIGQUIT, NULL);

    handlerSIGCHLD.sa_handler = signal_handler_SIGCHLD;
    sigemptyset(&(handlerSIGCHLD.sa_mask));
    handlerSIGCHLD.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &handlerSIGCHLD, NULL);
}

void travelMonitor::start(int m, int b, int s, string dir) {
    this->numMonitors = m;
    this->bufferSize = b;
    this->sizeOfBloom = s;
    this->input_dir = dir;
    // cout << "numMonitors=" << this->numMonitors << ", bufferSize= " << this->bufferSize << ", sizeOfBloom= " << this->sizeOfBloom << ", input_dir= " << this->input_dir << endl;
    this->countryToMonitor = NULL;
    this->monitors = NULL;
    this->viruses = new stringList();
    checkNew(this->viruses);

    this->blooms = new bloomFilterList(this->sizeOfBloom);
    checkNew(this->blooms);
}

void travelMonitor::createFIFOs() {
    string directory = "pipes/";
    for (int i = 0;i < numMonitors;i++) {
        string fullname = "pipes/fifo_tW_mR_" + to_string(i);
        char* fifoName = &fullname[0];

        if (mkfifo(fifoName, 0666) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                exit(EXIT_FAILURE);
            }
        }

        fullname = "pipes/fifo_tR_mW_" + to_string(i);
        fifoName = &fullname[0];

        if (mkfifo(fifoName, 0666) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

void travelMonitor::createMonitors() {
    for (int i = 0;i < numMonitors;i++) {
        createMonitor(i);
    }
}
void travelMonitor::createMonitor(int i) {
    pid_t c_pid = fork();
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (c_pid > 0) {
        cout << "Monitor " << i << " Created with pid " << c_pid << endl;
        this->addMonitor(c_pid, i);
    }
    else {
        string pipe0 = "pipes/fifo_tW_mR_" + to_string(i);
        string pipe1 = "pipes/fifo_tR_mW_" + to_string(i);

        execlp("./Monitor", pipe0.c_str(), pipe1.c_str(), NULL);
    }
}


void travelMonitor::openFifos() {
    for (int i = 0;i < numMonitors;i++) {
        openFifo(i);
    }
}
void travelMonitor::openFifo(int i) {

    string pipe0 = "pipes/fifo_tW_mR_" + to_string(i);
    string pipe1 = "pipes/fifo_tR_mW_" + to_string(i);
    int fd0 = open(pipe0.c_str(), O_WRONLY);
    int fd1 = open(pipe1.c_str(), O_RDONLY);
    // cout << "i=" << i << ",writefd=" << fd0 << ",readfd=" << fd1 << endl;
    this->addFdToMonitor(i, fd1, fd0);

}

void travelMonitor::sendCredentials() {
    for (int i = 0;i < numMonitors;i++) {
        sendCredential(i);
    }
}

void travelMonitor::sendCredential(int i) {

    int fd = this->monitors->getWriteFifo(i);
    // cout << "sendCredential i=" << i << ",writefd=" << fd << endl;
    if (write(fd, &i, sizeof(int)) == -1)
        cout << "Error in writting id with errno=" << errno << endl;
    if (write(fd, &this->bufferSize, sizeof(int)) == -1)
        cout << "Error in writting bufferSize with errno=" << errno << endl;
    if (write(fd, &this->sizeOfBloom, sizeof(int)) == -1)
        cout << "Error in writting sizeOfBloom with errno=" << errno << endl;

    sendStr(i, this->input_dir);

}

void travelMonitor::sendCountries() {
    int monitor = 0;
    int count;

    struct dirent** coutriesDir;
    count = scandir(this->input_dir.c_str(), &coutriesDir, NULL, alphasort);
    if (count < 0)
        perror("error in scandir");
    else {
        for (int i = 0;i < count;i++) {
            string country = coutriesDir[i]->d_name;
            if (country.compare("..") == 0 || country.compare(".") == 0) {
                free(coutriesDir[i]);
                continue;
            }

            sendCountry(country, monitor, true);

            monitor++;
            if (monitor >= this->numMonitors)
                monitor = 0;
            free(coutriesDir[i]);
        }
        free(coutriesDir);
    }

    int fd;
    for (int i = 0;i < this->numMonitors;i++) {
        fd = this->monitors->getWriteFifo(i);
        int end = -1;
        if (write(fd, &end, sizeof(int)) == -1)
            cout << "Error in writting end with errno=" << errno << endl;
    }
}
void travelMonitor::sendCountry(string country, int monitor, bool first) {
    // cout << country << "->" << monitor << endl;
    if (first)
        this->addCountryToMonitor(country, monitor);

    cout << "Sending to monitor " << monitor << " the country " << country << endl;
    sendStr(monitor, country);
}

void travelMonitor::receiveBlooms() {
    fd_set fileDecriptorSet;

    int totalRead = 0;
    int flag = 0;
    while (!flag) {
        FD_ZERO(&fileDecriptorSet);
        for (int i = 0;i < this->numMonitors;i++) {
            FD_SET(this->monitors->getReadFifo(i), &fileDecriptorSet);
        }
        select(this->monitors->getReadFifo(this->numMonitors - 1) + 1, &fileDecriptorSet, NULL, NULL, NULL);

        for (int i = 0;i < this->numMonitors;i++) {
            if (FD_ISSET(this->monitors->getReadFifo(i), &fileDecriptorSet)) {
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

void travelMonitor::receiveBlooms(int i) {
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
        int fd = this->monitors->getReadFifo(i);
        char* bloomArray = this->blooms->getBloom(this->viruses->search(virus))->getArray();
        for (int i = 0;i <= this->sizeOfBloom / this->bufferSize;i++) {
            char buff[this->bufferSize];
            if (read(fd, &buff, this->bufferSize) == -1)
                cout << "Error in reading bit with errno=" << errno << endl;

            for (int i = 0; i < this->bufferSize;i++)
                bloomArray[pos + i] = bloomArray[pos + i] | buff[i];

            pos += this->bufferSize;
        }
        // cout << virus << " ";
        // this->blooms->getBloom(this->viruses->search(virus))->print();
    }
}

void travelMonitor::sendDone() {
    for (int i = 0;i < numMonitors;i++)
        sendStr(i, "DONE");
}

void travelMonitor::startMenu() {
    while (1)
    {
        string input = getInput("Enter command: ");
        // int pid = this->monitors->getPID(1);
        // kill(pid, SIGUSR1);
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
                kill(getpid(), SIGINT);
            }
            else
                cout << "Invalid command!" << endl;
        }
        delete[] command;
    }
}


void travelMonitor::travelRequest(string* arguments, int length) {
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
        if (v != NULL)
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
            if (countryToMonitor->search(countryFrom) != NULL) {
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

void travelMonitor::travelStats(string* arguments, int length) {
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
        while (temp != NULL) {
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
        while (temp != NULL) {
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

void travelMonitor::addVaccinationRecords(string* arguments, int length) {
    cout << endl;
    cout << "- Selected: /addVaccinationRecords" << endl;

    if (length == 2) {
        string country = arguments[1];
        cout << "- country: " << country << endl;
        if (this->countryToMonitor->search(country) != NULL) {
            // sendStr(this->countryToMonitor->search(country)->getMonitor(), "break");
            this->sendSIGUSR1(this->countryToMonitor->search(country)->getMonitor());
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

void travelMonitor::searchVaccinationStatus(string* arguments, int length) {
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
            if (read(this->monitors->getReadFifo(i), &buff, sizeof(int)) == -1)
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

void travelMonitor::sendStr(int monitor, string str) {
    int fd = this->monitors->getWriteFifo(monitor);
    char* to_tranfer = &str[0];
    int sizeOfStr = strlen(to_tranfer);

    if (write(fd, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Error in writting sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr > this->bufferSize) {
        int pos = 0;
        for (int i = 0;i <= strlen(to_tranfer) / this->bufferSize;i++) {
            if (write(fd, &to_tranfer[pos], this->bufferSize) == -1)
                if (errno != 4)
                    cout << "Error in writting to_tranfer with errno=" << errno << endl;

            pos += this->bufferSize;
        }
    }
    else
        if (write(fd, &to_tranfer[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;

}

string travelMonitor::receiveStr(int monitor) {
    int fd = this->monitors->getReadFifo(monitor);
    int sizeOfStr;
    if (read(fd, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Error in reading sizeOfStr with errno=" << errno << endl;

    string str = "";
    if (sizeOfStr > this->bufferSize) {
        for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
            char buff[this->bufferSize + 1];
            if (read(fd, &buff[0], this->bufferSize) == -1)
                if (errno != 4)
                    cout << "Error in reading buff with errno=" << errno << endl;
            buff[this->bufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(fd, &buff[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

string travelMonitor::receiveManyStr(int monitor, int* end) {
    int fd = this->monitors->getReadFifo(monitor);
    int sizeOfStr;
    if (read(fd, &sizeOfStr, sizeof(int)) == -1)
        if (errno != 4)
            cout << "Error in reading sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr == -1) {
        *end = -1;
        return "";
    }

    string str = "";
    if (sizeOfStr > this->bufferSize) {
        for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
            char buff[this->bufferSize + 1];
            if (read(fd, &buff[0], this->bufferSize) == -1)
                if (errno != 4)
                    cout << "Error in reading buff with errno=" << errno << endl;
            buff[this->bufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(fd, &buff[0], sizeOfStr) == -1)
            if (errno != 4)
                cout << "Error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

void travelMonitor::addCountryToMonitor(string c, int m) {
    if (this->countryToMonitor == NULL)
        this->countryToMonitor = new monitorCountryPairList(c, m);
    else
        this->countryToMonitor = this->countryToMonitor->add(c, m);
}

void travelMonitor::addRequest(string c, string v, date dt, bool s) {
    if (this->requests == NULL)
        this->requests = new statsList(c, v, dt, s);
    else
        this->requests = this->requests->add(c, v, dt, s);
}

void travelMonitor::addNewVirus(string virusName)
{
    if (this->viruses->search(virusName) == NULL) // if we dont have that virus add it to the list of viruses
    {                                         // and make the bloom filter and the skiplist for that virus
        this->viruses = this->viruses->add(virusName);
        this->blooms = this->blooms->add(this->viruses);
    }
}

void travelMonitor::addMonitor(int pid, int id) {
    if (this->monitors == NULL)
        this->monitors = new monitorList(pid, id);
    else
        this->monitors->add(pid, id);
}

void travelMonitor::addFdToMonitor(int m, int r, int w) {
    this->monitors->addFD(m, r, w);
}

void travelMonitor::printAllViruses() {
    cout << "ALL THE viruses: ";
    this->viruses->print();
    cout << endl;
}

void travelMonitor::printCountryToMonitor() {
    this->countryToMonitor->print();
}