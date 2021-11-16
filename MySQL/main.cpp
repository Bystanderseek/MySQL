#include <iostream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <functional>
#include <iomanip>
#include "MySQL.h"

using namespace std;

int main()
{
    MySQL::login_show();
    while(true)
    {
        MySQL mySql;
        string line;
        string order("mysql>");
        cout << order;
        getline(cin, line);
        mySql.program_interface(line);
    }
}
