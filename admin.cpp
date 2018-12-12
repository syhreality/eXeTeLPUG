#include <fstream>
#include <iostream>
#include <string>
#include "admin.h"
#include "config.h"
using namespace std;

ADMIN::ADMIN()
{
    count = 0;
}

ADMIN::~ADMIN()
{
    clearData();
}

void ADMIN::setCount()
{
    char c;
    char old = '\0';
    ifstream ins;

    ins.open("admins.cfg");

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

void ADMIN::loadData()
{
    ifstream ins;

    setCount();
    ins.open("admins.cfg");

    if (ins.good())
    {
        for (int i = 0; i < count; i++)
        {
            ins >> tmp.steamid;
            ins >> tmp.name;
            db.push_back(tmp);
        }
    }
    else
    {
        cerr << "Error reading the file admins.cfg" << endl;
        exit(1);
    }
    ins.close();

    return;
}

void ADMIN::printData()
{
	cout << "\nCurrent Admins: " << endl << endl;
    for (vector<adminType>::iterator dbi = db.begin(); dbi != db.end(); dbi++)
    {
		cout << dbi->name << ":\t";
		if (dbi->name.size() < 7)
			cout << "\t";
		cout << dbi->steamid << endl;
    }
    cout << endl;
}

void ADMIN::printData(char adminlist[])
{
    vector<adminType>::iterator dbi;
    string list;

    list += "Current Admins: ";
    for (dbi = db.begin(); dbi != db.end() - 1; dbi++)
    {
        list += dbi->name;
        list += ", ";
    }
    list += dbi->name;
    list += ".\0";
    strcpy(adminlist, list.c_str());

    return;
}

void ADMIN::clearData()
{
    db.clear();

    return;
}

bool ADMIN::found(char id[])
{
    for (vector<adminType>::iterator dbi = db.begin(); dbi != db.end(); dbi++)
    {
        if (!strcmp(id, dbi->steamid.c_str()))
            return true;
    }
    return false;
}

bool ADMIN::addAdmin(char name[], char steamid[])
{
    if (!strncmp(steamid, "STEAM_", 6))
    {
        tmp.name = name;
        tmp.steamid = steamid;
        db.push_back(tmp);
        writeOut();
        return true;
    }

    return false;
}

bool ADMIN::delAdmin(char name[])
{
    for (vector<adminType>::iterator dbi = db.begin(); dbi != db.end(); dbi++)
    {
        if (!strcmpi(dbi->name.c_str(), name))
        {
            db.erase(dbi);
            writeOut();
            return true;
        }
    }
    return false;
}

void ADMIN::writeOut()
{
    ofstream outs;

    outs.open("admins.cfg");

    for (vector<adminType>::iterator dbi = db.begin(); dbi != db.end(); dbi++)
    {
        outs << dbi->steamid << "\t";
        if (dbi->steamid.size() < 16) {outs << "\t"; }
        outs << dbi->name << endl;
    }
    outs.close();

    return;
}
