#include <direct.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include "Database.h"
#include "MySQL.h"
#include "utility_func.h"

Database::Database()
{
    std::ifstream is(basic_path + table_directory);
    if(!is)   return;   //若没有表文件则就此返回
    std::string table_path;
    while(is >> table_path)
    {
        std::ifstream table_file(basic_path + table_path + "\\property.txt");//打开属性文件
        if(!table_file)
        {
            error_msg("Fail to open file");
        }
        std::vector<std::string> name;
        std::vector<std::string> type;
        name.reserve(5);    //事先分配好估计的大小，减少性能开销
        name.reserve(5);
        std::string primary_key;    //优先读入主键
        table_file >> primary_key;
        std::string value;          //表中的数据项
        bool reading_name = true;
        while(table_file >> value)
        {
            if(reading_name)
            {
                name.emplace_back(value);
                reading_name = false;
            }
            else
            {
                type.emplace_back(value);
                reading_name = true;
            }
        }
        table_file.close();
        if(primary_key == "NULL")   primary_key = "";
        tables.insert({table_path, Table(table_path, name, type,
                                         primary_key, true)});
        table_file.open(basic_path + table_path + "\\data.txt");
        if(!table_file)     //如果不存在数据文件，那就直接建表
        {
            continue;
        }
        else
        {
            std::vector<std::string> data_values;   //读data数据项的文件
            std::string line;
            while(std::getline(table_file, line))
            {
                std::stringstream ss(line);
                while (ss >> value)
                {
                    data_values.emplace_back(value);
                }
                tables.at(table_path).insert(data_values, true);
                data_values.clear();
            }
            table_file.close();
        }
    }
    is.close();
}

void Database::create_table(const std::string &table_name, std::vector<std::string> &name,
                       std::vector<std::string> &type, const std::string &primarykey)
{
    if(tables.find(table_name) == tables.cend()) //如果记录中不存在该表
    {
        tables.insert({table_name, Table(table_name, name, type, primarykey)});
        create_table_file(table_name);
        complete_msg();
    }
    else
    {
        fail_msg("Table " + table_name + " already exists");
    }
}

void Database::drop_table(const std::string &table_name)
{
    if(tables.erase(table_name) == 0)
    {
        fail_msg("Table " + table_name + " not exists");
    }
    else
    {
        delete_table_file(table_name);
        complete_msg("Table deleted");
    }
}

void Database::insert(const std::string &table_name, std::vector<std::string> &values)
{
    if(tables.find(table_name) == tables.end())
    {
        fail_msg("table " + table_name + " does not exist");
        return;
    }
    else
    {
        tables.at(table_name).insert(values);
        return;
    }
}

void Database::create_table_file(const std::string &table_name)
{
    std::ofstream os(basic_path + "tables.txt", std::ofstream::app);
    os << table_name << std::endl;
    os.close();
}

void Database::delete_table_file(const std::string &table_name)
{
    std::ofstream os(basic_path + "tables.txt");
    for(const auto &r : tables)
    {
        os << r.first << std::endl;
    }
    delete_directory_all(table_name);
    rmdir((basic_path + table_name).c_str());
}

void Database::desc(const std::string &table_name)
{
    if(tables.find(table_name) == tables.cend())
    {
        fail_msg("Table " + table_name + " doesn't exist");
        return;
    }
    tables.at(table_name).desc();
}

void Database::show_tables()
{
    if(tables.empty())
        fail_msg("Set is empty");
    std::vector<std::vector<std::string>> vvcs;
    vvcs.emplace_back(std::vector<std::string>{"Tables"});
    for(const auto &r : tables)
    {
        vvcs.emplace_back(std::vector<std::string>{r.first});
    }
    print(vvcs);
}

void Database::delete_(const std::string &table_name, const std::vector<std::string> &expression)
{
    if(tables.find(table_name) != tables.cend())
    {
        tables.at(table_name).delete_(expression);
    }
    else
    {
        fail_msg("Table " + table_name + " doesn't exist");
        return;
    }
}

void Database::delete_table_all(const std::string &table_name)
{
    if(tables.find(table_name) != tables.cend())
    {
        tables.at(table_name).delete_all();
    }
    else
    {
        fail_msg("Table " + table_name + " doesn't exist");
        return;
    }
}

void Database::select(const std::vector<std::string> &target, const std::vector<std::string> &relations)
{
    if(relations.size() == 2 && relations[0] == relations[1])   //两个名字重复
    {
        error_msg();
        return;
    }
    for(const auto &r : relations)
    {
        if(tables.find(r) == tables.cend())
        {
            fail_msg("Table " + r + " doesn't exist");
            return;
        }
    }
    for(const auto &r : relations)
    {
        tables.at(r).select_print(target);
    }
}

void Database::select(const std::vector<std::string> &target, const std::vector<std::string> &relations,
                      const std::vector<std::string> &condition)
{
    auto iter = std::find(condition.begin(), condition.end(), "and");
    if(iter != condition.cend())    //有两个条件
    {
        std::vector<std::string> vcs_1(condition.cbegin(), iter);    //条件1
        std::vector<std::string> vcs_2(++iter, condition.cend());    //条件2
        if(vcs_1.empty() || vcs_2.empty())  //有and但是却不足两个条件，比如and在末尾
        {
            error_msg();
            return;
        }
        for(const auto &r : relations)  //判断表是否存在
        {
            if(tables.find(r) == tables.cend())
            {
                error_msg("Table " + r + " doesn't exist");
                return;
            }
        }
        if(relations.size() == 1)   //只有一个表
        {
            std::vector<std::string> cd_1(get_valid_condition(vcs_1));
            std::vector<std::string> cd_2(get_valid_condition(vcs_2));
            if(cd_1.empty() || cd_2.empty())    //如果其中任何一个不符合语法规则
            {
                error_msg();
                return;
            }
            std::string table_name = relations[0];
            std::vector<int> index_1 = Table::select(tables.at(table_name), cd_1);
            std::vector<int> index_2 = Table::select(tables.at(table_name), cd_2);
            if((!index_1.empty() && index_1[0] == -1) || (!index_2.empty() && index_2[0] == -1))
            {
                //如果进入这条判断条件，说明条件表达式出错
                return;
            }
            else if(index_1.empty() || index_2.empty())
            {
                fail_msg("Empty set");
                return;
            }
            else
            {
                std::vector<int> result;
                std::set_intersection(index_1.begin(), index_1.end(), index_2.begin(),index_2.end(),
                          std::insert_iterator<std::vector<int>>(result, result.begin())); //求交集
                if(result.empty())  //判断结果是否为空
                {
                    fail_msg("Empty set");
                    return;
                }
                tables.at(table_name).select_print(target, result);
            }
        }
        else    //有两个表
        {
            std::vector<std::string> cd_1(get_valid_condition_specific(vcs_1));
            std::vector<std::string> cd_2(get_valid_condition_specific(vcs_2));
            if(cd_1.empty() || cd_2.empty())    //如果其中任何一个不符合语法规则
            {
                error_msg();
                return;
            }
            if(relations[0] == relations[1])    //两表名相同
            {
                error_msg();
                return;
            }
            std::vector<std::pair<int, int>> index = Table::select(tables.at(relations[0]),
                                                                   tables.at(relations[1]),
                                                                   cd_1, cd_2);
            if(index.empty())
            {
                fail_msg("Empty set");
                return;
            }
            else if(index[0].first == -1 || index[0].second == -1)
            {
                return; //说明出错了
            }
            Table::select_print(target, tables.at(relations[0]), tables.at(relations[1]), index);
        }
    }
    else    //只有一个条件
    {
        std::vector<std::string> vcs(condition.cbegin(), iter);
        std::vector<std::string> cd;
        if(relations.size() == 1)   //单表的选择条件
            cd = (get_valid_condition(vcs));
        else        //双表的连接条件
            cd = (get_valid_condition(vcs, true));
        if(cd.empty())  //cd为空说明条件表达式语法错误
        {
            error_msg();
            return;
        }
        for(const auto &r : relations)  //判断表是否存在
        {
            if(tables.find(r) == tables.cend())
            {
                error_msg("Table " + r + " doesn't exist");
                return;
            }
        }
        if(relations.size() == 1)   //一个表的判断条件
        {
            std::string table_name = relations[0];
            std::vector<int> index = Table::select(tables.at(table_name), cd);
            if(index.empty())
            {
                fail_msg("Empty set");
                return;
            }
            else if(index[0] == -1) //返回-1说明中途出错了
            {
                return;
            }
            tables.at(table_name).select_print(target, index);
        }
        else    //两个表一个连接条件
        {
            if(relations[0] == relations[1])    //两表名相同
            {
                error_msg();
                return;
            }
            std::vector<std::pair<int, int>> index = Table::select(tables.at(relations[0]),
                                                   tables.at(relations[1]), cd);
            if(index.empty())
            {
                fail_msg("Empty set");
                return;
            }
            else if(index[0].first == -1 || index[0].second == -1)
            {
                return; //说明出错了
            }
            Table::select_print(target, tables.at(relations[0]), tables.at(relations[1]), index);
        }
    }
}

std::vector<std::string> Database::get_valid_condition(const std::vector<std::string> &condition,
                                                       bool is_double)
{
    //如果is_double为假说明只判断单表的，是单表的属性之间的 =，<，>（如果为真则说明它是表的连接条件）
    if(condition.size() == 3)
    {
        if(!MySQL::is_valid_name(condition[0]))    //如果第一个是非法名称
        {
            return std::vector<std::string>();
        }
        if(condition[1] != "=" && condition[1] != "<" && condition[1] != ">")
        {
            return std::vector<std::string>();
        }
        if(is_double)   //单表的判断结束于此
        {
            if(!MySQL::is_valid_name(condition[2])) //如果第二个是非法名称
            {
                return std::vector<std::string>();
            }
            if(condition[0] == condition[2])    //如果两表要判断的属性名相同，不合法
            {
                return std::vector<std::string>();
            }
            return condition;   //相比于单表，不添加任何后缀
        }
        if(!is_number(condition[2]))    //第三位不是数字，语法错误
        {
            return std::vector<std::string>();
        }
        //以下是单表的处理
        std::vector<std::string> result(condition);
        result[0].append("i");   //做标记
        return result;
    }
    else if(condition.size() == 5) //说明是属性字符串与字符串的比较
    {
        if(!MySQL::is_valid_name(condition[0]) || condition[1] != "=" || condition[2] != "'" ||
                condition[4] != "'")
        {
            return std::vector<std::string>();
        }
        std::vector<std::string> result;
        result.emplace_back(condition[0]);
        result.emplace_back(condition[1]);
        result.emplace_back(condition[3]);
        result[0].append("c");  //做标记
        return result;
    }
    else
    {
        return std::vector<std::string>();
    }
}

std::vector<std::string> Database::get_valid_condition_specific(const std::vector<std::string> &condition)
{
    if(condition.size() == 3)
    {
        if(!MySQL::is_valid_name(condition[0]))    //如果第一个是非法名称
        {
            return std::vector<std::string>();
        }
        if(condition[1] != "=" && condition[1] != "<" && condition[1] != ">")
        {
            return std::vector<std::string>();
        }
        if(MySQL::is_valid_name(condition[2])) //能够通过这个判断说明它是连接条件
        {
            if(condition[0] == condition[2])    //如果两表要判断的属性名相同，不合法
            {
                return std::vector<std::string>();
            }
            return condition;   //相比于int的判断表它不添加任何后缀
        }
        else    //通过这个条件说明它不是连接条件，是int的表达式的判断
        {
            if(!is_number(condition[2]))
            {
                error_msg();
                return std::vector<std::string>();
            }
            std::vector<std::string> result(condition);
            result[0].append("i");   //做标记
            result.emplace_back("-1");  //长度大于3说明它不是连接条件
            return result;
        }
    }
    else if(condition.size() == 5) //说明是属性字符串与字符串的比较，说明它不是连接条件，添加后缀
    {
        if(!MySQL::is_valid_name(condition[0]) || condition[1] != "=" || condition[2] != "'" ||
           condition[4] != "'")
        {
            error_msg();
            return std::vector<std::string>();
        }
        std::vector<std::string> result;
        result.emplace_back(condition[0]);
        result.emplace_back(condition[1]);
        result.emplace_back(condition[3]);
        result[0].append("c");  //做标记
        result.emplace_back("-1");
        return result;
    }
    else
    {
        return std::vector<std::string>();
    }
}
