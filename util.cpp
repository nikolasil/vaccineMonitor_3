#include <iostream>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"

int checkArguments(int argc, char* argv[], int& numMonitors, int& bufferSize, int& sizeOfBloom, string& input_dir)
{
    if (argc != 9)
    {
        cout << "ERROR The number of arguments given must be 8" << endl;
        return 0;
    }
    struct stat buffer;
    string args[4] = { "-m", "-b", "-s", "-i" };
    int pos = 1;
    while (1) {
        if (pos > 7) {
            // cout << "Found the 4 args" << endl;
            break;
        }
        int i = 0;
        for (i = 0; i < 4; i++) {
            // cout << "pos=" << pos << ",i=" << i << ",arg=" << string(argv[pos]) << ",array=" << args[i] << endl;
            if (string(argv[pos]).compare(args[i]) == 0) {
                args[i] = ""; // clear it for future checking
                if (string(argv[pos]) == "-m") {
                    // cout << "-m" << endl;

                    string c = string(argv[pos + 1]);
                    std::string::const_iterator it = c.begin();
                    while (it != c.end() && isdigit(*it))
                        ++it;

                    bool check = !c.empty() && it == c.end();

                    if (atoi(argv[pos + 1]) <= 0 || !check)
                    {
                        cout << "ERROR The numMonitors must be unsigned integer" << endl;
                        return 0;
                    }
                    numMonitors = atoi(argv[pos + 1]);
                }
                else if (string(argv[pos]) == "-b") {
                    // cout << "-b" << endl;

                    string c = string(argv[pos + 1]);
                    std::string::const_iterator it = c.begin();
                    while (it != c.end() && isdigit(*it))
                        ++it;

                    bool check = !c.empty() && it == c.end();

                    if (atoi(argv[pos + 1]) <= 0 || !check)
                    {
                        cout << "ERROR The bufferSize must be unsigned integer" << endl;
                        return 0;
                    }
                    bufferSize = atoi(argv[pos + 1]);
                }
                else if (string(argv[pos]) == "-s") {
                    // cout << "-s" << endl;

                    string c = string(argv[pos + 1]);
                    std::string::const_iterator it = c.begin();
                    while (it != c.end() && isdigit(*it))
                        ++it;

                    bool check = !c.empty() && it == c.end();

                    if (atoi(argv[pos + 1]) <= 0 || !check)
                    {
                        cout << "ERROR The sizeOfBloom must be unsigned integer" << endl;
                        return 0;
                    }
                    sizeOfBloom = atoi(argv[pos + 1]);
                }
                else if (string(argv[pos]) == "-i") {
                    // cout << "-i" << endl;
                    if (stat(argv[pos + 1], &buffer) != 0)
                    {
                        cout << "ERROR The input_dir=" << argv[pos + 1] << " don't exists" << endl;
                        return 0;
                    }
                    input_dir = string(argv[pos + 1]);
                }
                else {
                    cout << "Error in arguments" << endl;
                }
                pos += 2;
                break;
            }
        }
        if (i == 4) {
            cout << "Error Arguments must start with -m,-b,-s and -i" << endl;
            break;
        }
    }
    return 1;
}

void checkNew(void* ptr)
{
    if (ptr == NULL)
    {
        cout << "errno = " << errno << endl;
        perror("errno");
        exit(EXIT_FAILURE);
    }
}

string getInput(string prompt)
{
    string value;
    cout << prompt;
    cin.seekg(0, ios::end);
    cin.clear();
    getline(cin, value);
    // cin.seekg(0, ios::end);
    // cin.clear();
    return value;
}

void splitString(string** words, string input, int* length)
{
    string arg;
    istringstream str(input);

    int i = 0;
    while (str >> arg)
    {
        if (i == 8) {
            *length = 9;
            return;
        }
        (*words)[i++] = arg;
    }
    if (i == 7) {
        (*words)[7] = "";
    }
    *length = i;
}

string* readString(string input, int* length)
{
    int i = 1;
    string arg;

    istringstream str(input);

    while (str >> arg)
    {
        i++;
    }
    *length = i - 1;
    istringstream str2(input);
    string* arguments = new string[i - 1];
    if (arguments == NULL)
    {
        cout << "readString errno = " << errno << endl;
        perror("errno");
        exit(EXIT_FAILURE);
    }
    i = 0;
    while (str2 >> arg)
    {
        arguments[i++] = arg;
    }
    return arguments;
}