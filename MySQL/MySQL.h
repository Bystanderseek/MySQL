#ifndef MYSQL_MYSQL_H
#define MYSQL_MYSQL_H
#include <iostream>
#include <set>
#include <map>
#include <utility>
#include <functional>
#include <vector>
#include "Database.h"

class MySQL
{
public:
    MySQL();
    void program_interface(std::string &line) const;
    static bool is_valid_name(const std::string& name);
    static void login_show();

    static std::vector<std::string>& get_vec(const std::string &line);
    static const std::set<std::string>& illegal_name();   //非法名称
private:
    static std::pair<bool, std::string> grammar_analyze(std::string &line);
    static void create(const std::string &line);
    static void select(const std::string &line);
    static void insert(const std::string &line);
    static void delete_(const std::string &line);   //只支持单关系的条件删除
    static void drop(const std::string &line);
    static void quit(const std::string &line);
    static void desc(const std::string &line);
    static void show(const std::string &line);
    void execute(const std::pair<bool, std::string> &line) const;
private:
    std::map<std::string, std::function<void (const std::string&)>> sql_operator;
    static Database database;
};


#endif //MYSQL_MYSQL_H
