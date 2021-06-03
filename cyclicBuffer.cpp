#include <iostream>
#include <string>
#include <unistd.h>

#include "cyclicBuffer.h"

using namespace std;

cyclicBuffer::cyclicBuffer(int s) : size(s) {
    cout << "cyclicBuffer constructor" << endl;
    this->buff = new string[this->size];
    this->start = 0;
    this->end = -1;
    this->count = 0;
    pthread_mutex_init(&(this->mtx), nullptr);
    pthread_cond_init(&(this->cond_nonempty), nullptr);
    pthread_cond_init(&(this->cond_nonfull), nullptr);
}

cyclicBuffer::~cyclicBuffer() {
    cout << "cyclicBuffer destructor" << endl;
    delete[] this->buff;
    pthread_cond_destroy(&(this->cond_nonempty));
    pthread_cond_destroy(&(this->cond_nonfull));
    pthread_mutex_destroy(&(this->mtx));
}

string cyclicBuffer::take() {
    string data = "";
    pthread_mutex_lock(&(this->mtx));
    while (this->count <= 0) {
        cout << "Buffer is empty" << endl;
        pthread_cond_wait(&(this->cond_nonempty), &(this->mtx));
    }
    data = this->buff[this->start];
    this->start = (this->start + 1) % this->size;
    this->count--;
    pthread_mutex_unlock(&(this->mtx));
    return data;
}

void cyclicBuffer::put(string txt) {
    pthread_mutex_lock(&(this->mtx));
    while (this->count >= this->size) {
        cout << "Buffer is full" << endl;
        pthread_cond_wait(&(this->cond_nonfull), &mtx);
    }
    this->end = (this->end + 1) % this->size;
    this->buff[this->end] = txt;
    this->count++;
    pthread_mutex_unlock(&(this->mtx));
}