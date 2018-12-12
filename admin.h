#ifndef ADMIN_H
#define ADMIN_H
#include <string>
#include <vector>
#include "config.h"
using namespace std;

struct adminType
{
    string name;
    string steamid;
};

class ADMIN
{
    public:
        ADMIN();
        ~ADMIN();
        void setCount();
        void loadData();
        void printData();
        void printData(char[]);
        void clearData();
        bool found(char[]);
        bool addAdmin(char[], char[]);
        bool delAdmin(char[]);
        void writeOut();
    private:
        vector<adminType> db;
        adminType tmp;
        int count;
};

#endif
