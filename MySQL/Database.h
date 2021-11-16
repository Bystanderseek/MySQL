#ifndef MYSQL_DATABASE_H
#define MYSQL_DATABASE_H
#include <iostream>
#include <fstream>
#include <direct.h>
#include "Table.h"
#include "File.h"

class Database
{
public:
    Database();
    void drop_table(const std::string &table_name);
    void create_table(const std::string &table_name, std::vector<std::string> &name,
                      std::vector<std::string> &type, const std::string& primarykey);
    void insert(const std::string &table_name, std::vector<std::string> &values);
    void desc(const std::string &table_name);
    void show_tables();
    void delete_(const std::string &table_name, const std::vector<std::string> &expression);
    void delete_table_all(const std::string &table_name);
    void select(const std::vector<std::string> &target, const std::vector<std::string> &relations);
    void select(const std::vector<std::string> &target, const std::vector<std::string> &relations,
                const std::vector<std::string> &condition);
private:
    void create_table_file(const std::string &table_name);
    void delete_table_file(const std::string &table_name);
    std::vector<std::string> get_valid_condition(const std::vector<std::string> &condition,
                                                 bool is_double = false);
    //����汾��get_valid_condition���������ͨ�ģ���������ȷ���ж�ĳ���ʽ�ǲ����������������ݷ��ص������Сȷ����
    static std::vector<std::string> get_valid_condition_specific(const std::vector<std::string> &condition);
    std::map<std::string, Table> tables;
    std::string table_directory = "tables.txt";
};


#endif //MYSQL_DATABASE_H
