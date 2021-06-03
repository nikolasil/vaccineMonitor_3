#ifndef CYCLICBUFFER_H
#define CYCLICBUFFER_H

#include <cstdlib>
#include <pthread.h>

#include <iostream>
#include <string>
#include <unistd.h>


class cyclicBuffer {
public:
    cyclicBuffer(int s);
    ~cyclicBuffer();


    void singalEmpty() { pthread_cond_signal(&cond_nonempty); }
    void singalFull() { pthread_cond_signal(&cond_nonfull); }

    string take();
    void put(string txt);

private:
    string* buff;
    int start;
    int end;
    int count;
    int size;
    pthread_mutex_t mtx;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
}

#endif