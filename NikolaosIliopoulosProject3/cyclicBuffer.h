#ifndef CYCLICBUFFER_H
#define CYCLICBUFFER_H

#include <cstdlib>
#include <pthread.h>

#include <iostream>
#include <string>
#include <unistd.h>


using namespace std;

class cyclicBuffer {
public:
    cyclicBuffer(int s, int txtNumber);
    cyclicBuffer(int s);
    ~cyclicBuffer();


    void singalEmpty() { pthread_cond_signal(&cond_nonempty); }
    void singalFull() { pthread_cond_signal(&cond_nonfull); }
    void waitEmpty() { pthread_cond_wait(&cond_nonempty, &(this->mtx)); }
    void waitFull() { pthread_cond_wait(&cond_nonfull, &(this->mtx)); }

    void increaseParsed() {
        pthread_mutex_lock(&(this->mtx));
        this->txtParsed++;
        pthread_mutex_unlock(&(this->mtx));
    }

    void setTxtNumber(int a) { this->txtNumber = a; }

    void finishedParsing() { while (this->txtParsed < this->txtNumber) {} }

    void reset();

    string take();
    void put(string txt);

private:
    string* buff;
    int start;
    int end;
    int count;
    int size;
    int txtNumber;
    int txtParsed;

    pthread_mutex_t mtx;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
};

#endif