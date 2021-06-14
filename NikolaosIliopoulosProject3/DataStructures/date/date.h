/*
    This file contains the date class that is used in the citizenRecord class.
*/

#ifndef DATE_H
#define DATE_H

#include <string>

using namespace std;

class date
{
public:
    date(string dd, string mm, string yy);
    date(string d);
    date();
    ~date();

    void setAll(string d);
    void print();
    int compare(date d);
    bool isValid(); // return true if the date is in correct format (dd-mm-yyyy)
    string getConcatenate();

    // GETTERS
    string getDay() { return this->day; }
    string getMonth() { return this->month; }
    string getYear() { return this->year; }

    // SETTERS
    void setDay(string d) { this->day = d; }
    void setMonth(string m) { this->month = m; }
    void setYear(string y) { this->year = y; }
    void setAll(string dd, string mm, string yy) { this->day = dd;this->month = mm;this->year = yy; }

private:
    string day;
    string month;
    string year;
};

#endif