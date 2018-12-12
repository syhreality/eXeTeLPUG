#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include "warning.h"
using namespace std;

WARNING::WARNING()
{
    count = 0;
}

WARNING::~WARNING()
{
    db.clear();
}

void WARNING::setCount()
{
    char c;
    char old = '\0';
    ifstream ins;

    ins.open("database.dat");

    while(ins.good())
    {
        c = ins.get();
        if ((c == '\n' && old != '\n') || (c == EOF && old != '\n'))
            count++;
        old = c;
    }
    ins.close();
}

void WARNING::loadData()
{
    char in[32];
    ifstream ins;

    setCount();
    ins.open("database.dat");
    if (ins.good())
    {
        for (int i = 0; i < count; i++)
        {
            ins >> tmp.id;
            ins >> tmp.warn;
            ins.ignore();
            ins.getline(in, 32, '\n');
            tmp.name = in;
            db.push_back(tmp);
        }
    }
    else
    {
        cerr << "Error reading the file database.dat" << endl;
        exit(1);
    }
    ins.close();

    return;
}

void WARNING::sortData()
{
    sort(db.begin(), db.end());

    return;
}

void WARNING::writeData()
{
    ofstream outs;

    outs.open("database.dat");
    if (outs.good())
    {
        for (vector<database>::iterator dbi = db.begin(); dbi != db.end(); dbi++)
            outs << dbi->id << " " << dbi->warn << " " << dbi->name << endl;
    }
    else
        cerr << "Error reading the file database.dat" << endl;
    outs.close();

    return;
}

int WARNING::updateData(const char id[], const char lname[])
{
    int index = -1;
    int warns = 1;
    string key = id;

    if (key == "STEAM_ID_PENDING")
        return 0;               // nip in the bud, this breaks our sort function

    if (findData(key, index))
    {
        db[index].name = lname; // update the player name
        db[index].warn++;       // increment the previous warning
        warns = db[index].warn;
    }
    else
    {
        tmp.id = key;           // add the STEAMID to id
        tmp.name = lname;       // store the player name
        tmp.warn = 1;           // set the initial warning to 1
        db.push_back(tmp);      // add the entry to the vector
        sortData();             // sort the vector
    }
    writeData();

    return warns;
}

bool WARNING::removeData(const char id[])
{
    int index = -1;
    string key = id;

    if (findData(key, index))
    {
        if (db[index].warn > 1)
            db[index].warn--;
        else
        {
            db.erase((db.begin()+index));
            sortData();
        }
        writeData();

        return true;
    }

    return false;
}

bool WARNING::findData(string key)
{
    if (binary_search(db.begin(), db.end(), tmp))
        return true;
    return false;
}

bool WARNING::findData(string key, int& index) // binary search implementation with index return
{
    int lowerbound = 0;
    int upperbound = db.size() - 1;
    int search_pos = (lowerbound + upperbound) / 2;
    index = -1;
    tmp.id = key;

    while ((db[search_pos].id != key) && (lowerbound <= upperbound))
    {
        if (db[search_pos] < tmp)
            lowerbound = search_pos + 1;
        else
            upperbound = search_pos - 1;
        search_pos = (lowerbound + upperbound) / 2;
    }

    if (lowerbound <= upperbound)
    {
        index = search_pos;
        return true;
    }
    return false;
}

bool database::operator<(const database &rhs) const
{
    if (this->id < rhs.id)
    {
        if (strlen(this->id.c_str()) > strlen(rhs.id.c_str()))
        {
            if (this->id[8] < rhs.id[8])
                return true; // ..0:0.. < ..0:1..
            return false; // strlen(str1) > strlen(str2)
        }
        return true; // str1 < str2
    }
    else
    {
        if (strlen(this->id.c_str()) < strlen(rhs.id.c_str()))
        {
            if (this->id[8] > rhs.id[8])
                return false; // ..0:1.. > ..0:0..
            return true;  // strlen(str1) < strlen(str2)
        }
        return false; // str2 > str1
    }
}
