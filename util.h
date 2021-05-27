#ifndef UTIL_H
#define UTIL_H

#include <string>

using namespace std;

int checkArguments(int argc, char* argv[], int& numMonitors, int& socketBufferSize, int& cyclicBufferSize, int& sizeOfBloom, string& input_dir, int& numThreads);
int checkArgumentsServer(int argc, char* argv[], int& port, int& numThreads, int& socketBufferSize, int& cyclicBufferSize, int& sizeOfBloom, char** paths);
void checkNew(void* ptr);
string getInput(string prompt);
void splitString(string** words, string input, int* length);
string* readString(string input, int* length);

#endif