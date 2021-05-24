#include <iostream>
#include <string>

#include "date.h"

using namespace std;

/*
- - - date Methods Iimplemantation - - -
*/

date::date(string dd, string mm, string yy) : day(dd), month(mm), year(yy) {}

date::date(string d)
{
    this->setAll(d);
}
date::date() {}

date::~date() {}

void date::print()
{
    cout << this->day << "-" << this->month << "-" << this->year;
}

string date::getConcatenate() {
    return this->day + "-" + this->month + "-" + this->year;
}

int date::compare(date d)
{
    string a;
    a.append(this->year);
    a.append("-");
    a.append(this->month);
    a.append("-");
    a.append(this->day);

    string b;
    b.append(d.year);
    b.append("-");
    b.append(d.month);
    b.append("-");
    b.append(d.day);

    if (a.compare(b) < 0)
        return -1;

    else if (a.compare(b) > 0)
        return 1;

    return 0;
}

bool date::isValid()
{
    std::string::const_iterator it = (this->day).begin();
    while (it != (this->day).end() && isdigit(*it))
        ++it;
    bool day = !(this->day).empty() && it == (this->day).end();

    it = (this->month).begin();
    while (it != (this->month).end() && isdigit(*it))
        ++it;
    bool month = !(this->month).empty() && it == (this->month).end();

    it = (this->year).begin();
    while (it != (this->year).end() && isdigit(*it))
        ++it;

    bool year = !(this->year).empty() && it == (this->year).end();
    if (day && month && year)
    {
        int d = stoi((this->day));
        int m = stoi((this->month));
        int y = stoi((this->year));
        if (d > 0 && d <= 30 && m > 0 && m <= 12 && y >= 1900 && y <= 2100)
            return true;
    }
    return false;
}

void date::setAll(string d)
{
    this->day = "";
    this->month = "";
    this->year = "";
    if (d.compare("") == 0)
        return;

    if (isdigit(d.at(0)) && isdigit(d.at(d.length() - 1)))
    {
        string delimiter = "-";
        d.append(delimiter);
        size_t pos = 0;
        string token;
        int i = 0;
        string arr[3];
        while ((pos = d.find(delimiter)) != string::npos)
        {
            if (i == 3)
                return;

            token = d.substr(0, pos);
            arr[i++] = token;
            d.erase(0, pos + delimiter.length());
        }

        this->day = arr[0];
        this->month = arr[1];
        this->year = arr[2];
    }
}