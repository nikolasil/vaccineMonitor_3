#include <iostream>
#include <string>
#include <unistd.h>

#include "cyclicBuffer.h"

using namespace std;


cyclicBuffer::cyclicBuffer(int s, int t) : size(s), txtNumber(t) {
    // cout << "cyclicBuffer constructor" << endl;
    this->buff = new string[this->size];
    this->start = 0;
    this->end = -1;
    this->count = 0;
    this->txtParsed = 0;

    pthread_mutex_init(&(this->mtx), nullptr);
    pthread_cond_init(&(this->cond_nonempty), nullptr);
    pthread_cond_init(&(this->cond_nonfull), nullptr);
}

cyclicBuffer::cyclicBuffer(int s) : size(s) {
    // cout << "cyclicBuffer constructor" << endl;
    this->buff = new string[this->size];
    for (int i = 0;i < this->size;i++)
        this->buff[i] = "";
    this->start = 0;
    this->end = -1;
    this->count = 0;
    this->txtParsed = 0;

    pthread_mutex_init(&(this->mtx), nullptr);
    pthread_cond_init(&(this->cond_nonempty), nullptr);
    pthread_cond_init(&(this->cond_nonfull), nullptr);
}

void cyclicBuffer::reset() {
    for (int i = 0;i < this->size;i++)
        this->buff[i] = "";
    this->start = 0;
    this->end = -1;
    this->count = 0;
    this->txtParsed = 0;
}

cyclicBuffer::~cyclicBuffer() {
    delete[] this->buff;
    pthread_cond_destroy(&(this->cond_nonempty));
    pthread_cond_destroy(&(this->cond_nonfull));
    pthread_mutex_destroy(&(this->mtx));
}

string cyclicBuffer::take() {
    pthread_mutex_lock(&(this->mtx));

    while (this->count <= 0)
        this->waitEmpty();

    string data = this->buff[this->start];
    this->start = (this->start + 1) % this->size;
    this->count--;
    // cout << "count-- = " << count << " take " << data << endl;
    pthread_mutex_unlock(&(this->mtx));
    return data;
}

void cyclicBuffer::put(string txt) {
    pthread_mutex_lock(&(this->mtx));

    while (this->count >= this->size)
        this->waitFull();

    this->end = (this->end + 1) % this->size;
    this->buff[this->end] = txt;
    this->count++;
    // cout << "count++ = " << count << " put " << txt << endl;
    pthread_mutex_unlock(&(this->mtx));
}