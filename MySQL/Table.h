#ifndef MYSQL_TABLE_H
#define MYSQL_TABLE_H
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include "File.h"

class Table
{
public:
    using property = std::pair<std::string, std::string>;

    Table(std::string Table_name, std::vector<std::string> &name, std::vector<std::string> &type,
          std::string Primary = "", bool read_file = false);
    void insert(std::vector<std::string> &values, bool read_file = false);
    void delete_(const std::vector<std::string> &expression);
    void delete_all();
    void desc();
    void select_print(const std::vector<std::string> &property_list) const;   //负责单表无条件的输出
    void select_print(const std::vector<std::string> &property_list,          //负责单表有条件的输出
                const std::vector<int> &index) const;
    //负责判断表中是否具有某个属性（不带类型检查）
    std::pair<bool, std::string> match(const std::string &property_name) const;
    //负责判断表中是否具有某个属性（带类型检查）
    bool match_with_type(const std::string &property_name) const;
    //负责单表条件的查找
    static std::vector<int> select(const Table &table, const std::vector<std::string> &condition);
    //负责双表单条件的查找
    static std::vector<std::pair<int, int>> select(const Table &table_1, const Table &table_2,
                                                   const std::vector<std::string> &condition);
    //负责双表双条件的查找
    static std::vector<std::pair<int, int>> select(const Table &table_1, const Table &table_2,
                                                   std::vector<std::string> &condition_1,
                                                   std::vector<std::string> &condition_2);
    //负责双表的输出
    static void select_print(const std::vector<std::string> &property_list, const Table &table_1,
                       const Table &table_2, const std::vector<std::pair<int, int>> &index);
private:

    static bool match(std::string& value, const std::string& type);
    static bool is_connect_condition(const Table &table_1, const Table &table_2,
                                     const std::vector<std::string> &condition);
    std::pair<bool, int> is_property_exist(const std::string &property_name, bool is_int);
    void write();

    std::vector<property> property_vc;                      //属性的数组
    std::vector<std::vector<std::string>> data;             //数据数组
    std::unordered_map<std::string, int> primary_key_table; //主键的哈希表
    std::map<std::string, int> property_index;              //存储属性名的下标的map
    std::string primary_key;
    std::string table_name;
    int primary_key_place;
};


#endif //MYSQL_TABLE_H
