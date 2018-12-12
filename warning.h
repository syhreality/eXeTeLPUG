#ifndef WARNING_H
#define WARNING_H
#include <string>
#include <vector>
#include "config.h"
using namespace std;

struct database
{
    string id;
    string name;
    int warn;
    bool operator<(const database &rhs) const;
};

class WARNING
{
    public:
        WARNING();                      // default constuctor
        ~WARNING();                     // destructor
        void setCount();                // counts how many real lines are in the file
        void loadData();                // loads the database from file
        void sortData();                // sorts the loaded database into order
        void writeData();               // writes the database to file
        int updateData(const char[], const char[]);   // the "do-work" function, adds 1, returns warn value
        bool removeData(const char[]);  // removes 1 from warnings if true, returns false if key doesn't exist
        bool findData(string);          // searches the database for a STEAMID
        bool findData(string, int&);    // searches the database for a STEAMID, returns index
    private:
        vector<database> db;            // the database
        database tmp;                   // temporary database type space
        int count;                      // how many real lines the file contains
        
};

#endif
