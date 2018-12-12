#ifndef MAP_H
#define MAP_H
#include <string>
#include <vector>
using namespace std;

class MAP
{
    public:
        MAP();
        ~MAP();
        void setCount();
        void loadData();
        void sortData();
        void writeData();
        void printData(char[]);
        void clearData();
        bool found(char[]);
        bool addMap(char[]);
        bool delMap(char[]);
    private:
        vector<string> db;
        int count;
};

#endif
