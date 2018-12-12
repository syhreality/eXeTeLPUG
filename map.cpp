#include <fstream>
#include <iostream>
#include "config.h"
#include "map.h"
using namespace std;

MAP::MAP()
{
    count = 0;
}

MAP::~MAP()
{
    clearData();
}

void MAP::setCount()
{
    char c;
    char old = '\0';
    ifstream ins;

    ins.open("maps.cfg");

    while(ins.good())
    {
        c = ins.get();
        if ((c == '\n' && old != '\n') || (c == EOF && old != '\n'))
            count++;
        old = c;
    }
    ins.close();

    return;
}

void MAP::loadData()
{
    ifstream ins;
    string tmp;

    setCount();
    ins.open("maps.cfg");

    if (ins.good())
    {
        for (int i = 0; i < count; i++)
        {
            ins >> tmp;
            db.push_back(tmp);
        }
    }
    else
    {
        cerr << "Error reading the file maps.cfg" << endl;
        exit(1);
    }

    return;
}

void MAP::sortData()
{
    sort(db.begin(), db.end());

    return;
}

void MAP::writeData()
{
    ofstream outs;

    outs.open("maps.cfg");

    vector<string>::iterator dbi;
    for (dbi = db.begin(); dbi != db.end(); dbi++)
        outs << *dbi << endl;
    outs.close();

    return;
}

void MAP::printData(char adminlist[])
{
    vector<string>::iterator dbi;
    string list;

    list += "Valid Maps: ";
    for (dbi = db.begin(); dbi != db.end() - 1; dbi++)
    {
        list += *dbi;
        list += ", ";
    }
    list += *dbi;
    list += ".\0";
    strcpy(adminlist, list.c_str());

    return;
}

void MAP::clearData()
{
    db.clear();

    return;
}

bool MAP::found(char id[])
{
    vector<string>::iterator dbi;

    for (dbi = db.begin(); dbi != db.end(); dbi++)
        if (!strcmp(id, (*dbi).c_str()))
            return true;
    return false;
}

bool MAP::addMap(char map[])
{
    if (!strncmp(map, "de_", 3))
    {
        db.push_back(map);
        sortData();
        writeData();
        return true;
    }
    return false;
}

bool MAP::delMap(char map[])
{
    vector<string>::iterator dbi;

    for (dbi = db.begin(); dbi != db.end(); dbi++)
    {
        if (!strcmpi((*dbi).c_str(), map))
        {
            db.erase(dbi);
            sortData();
            writeData();
            return true;
        }
    }
    return false;
}
