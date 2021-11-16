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
    if(!is)   return;   //��û�б��ļ���ʹ˷���
    std::string table_path;
    while(is >> table_path)
    {
        std::ifstream table_file(basic_path + table_path + "\\property.txt");//�������ļ�
        if(!table_file)
        {
            error_msg("Fail to open file");
        }
        std::vector<std::string> name;
        std::vector<std::string> type;
        name.reserve(5);    //���ȷ���ù��ƵĴ�С���������ܿ���
        name.reserve(5);
        std::string primary_key;    //���ȶ�������
        table_file >> primary_key;
        std::string value;          //���е�������
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
        if(!table_file)     //��������������ļ����Ǿ�ֱ�ӽ���
        {
            continue;
        }
        else
        {
            std::vector<std::string> data_values;   //��data��������ļ�
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
    if(tables.find(table_name) == tables.cend()) //�����¼�в����ڸñ�
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
    if(relations.size() == 2 && relations[0] == relations[1])   //���������ظ�
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
    if(iter != condition.cend())    //����������
    {
        std::vector<std::string> vcs_1(condition.cbegin(), iter);    //����1
        std::vector<std::string> vcs_2(++iter, condition.cend());    //����2
        if(vcs_1.empty() || vcs_2.empty())  //��and����ȴ������������������and��ĩβ
        {
            error_msg();
            return;
        }
        for(const auto &r : relations)  //�жϱ��Ƿ����
        {
            if(tables.find(r) == tables.cend())
            {
                error_msg("Table " + r + " doesn't exist");
                return;
            }
        }
        if(relations.size() == 1)   //ֻ��һ����
        {
            std::vector<std::string> cd_1(get_valid_condition(vcs_1));
            std::vector<std::string> cd_2(get_valid_condition(vcs_2));
            if(cd_1.empty() || cd_2.empty())    //��������κ�һ���������﷨����
            {
                error_msg();
                return;
            }
            std::string table_name = relations[0];
            std::vector<int> index_1 = Table::select(tables.at(table_name), cd_1);
            std::vector<int> index_2 = Table::select(tables.at(table_name), cd_2);
            if((!index_1.empty() && index_1[0] == -1) || (!index_2.empty() && index_2[0] == -1))
            {
                //������������ж�������˵���������ʽ����
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
                          std::insert_iterator<std::vector<int>>(result, result.begin())); //�󽻼�
                if(result.empty())  //�жϽ���Ƿ�Ϊ��
                {
                    fail_msg("Empty set");
                    return;
                }
                tables.at(table_name).select_print(target, result);
            }
        }
        else    //��������
        {
            std::vector<std::string> cd_1(get_valid_condition_specific(vcs_1));
            std::vector<std::string> cd_2(get_valid_condition_specific(vcs_2));
            if(cd_1.empty() || cd_2.empty())    //��������κ�һ���������﷨����
            {
                error_msg();
                return;
            }
            if(relations[0] == relations[1])    //��������ͬ
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
                return; //˵��������
            }
            Table::select_print(target, tables.at(relations[0]), tables.at(relations[1]), index);
        }
    }
    else    //ֻ��һ������
    {
        std::vector<std::string> vcs(condition.cbegin(), iter);
        std::vector<std::string> cd;
        if(relations.size() == 1)   //�����ѡ������
            cd = (get_valid_condition(vcs));
        else        //˫�����������
            cd = (get_valid_condition(vcs, true));
        if(cd.empty())  //cdΪ��˵���������ʽ�﷨����
        {
            error_msg();
            return;
        }
        for(const auto &r : relations)  //�жϱ��Ƿ����
        {
            if(tables.find(r) == tables.cend())
            {
                error_msg("Table " + r + " doesn't exist");
                return;
            }
        }
        if(relations.size() == 1)   //һ������ж�����
        {
            std::string table_name = relations[0];
            std::vector<int> index = Table::select(tables.at(table_name), cd);
            if(index.empty())
            {
                fail_msg("Empty set");
                return;
            }
            else if(index[0] == -1) //����-1˵����;������
            {
                return;
            }
            tables.at(table_name).select_print(target, index);
        }
        else    //������һ����������
        {
            if(relations[0] == relations[1])    //��������ͬ
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
                return; //˵��������
            }
            Table::select_print(target, tables.at(relations[0]), tables.at(relations[1]), index);
        }
    }
}

std::vector<std::string> Database::get_valid_condition(const std::vector<std::string> &condition,
                                                       bool is_double)
{
    //���is_doubleΪ��˵��ֻ�жϵ���ģ��ǵ��������֮��� =��<��>�����Ϊ����˵�����Ǳ������������
    if(condition.size() == 3)
    {
        if(!MySQL::is_valid_name(condition[0]))    //�����һ���ǷǷ�����
        {
            return std::vector<std::string>();
        }
        if(condition[1] != "=" && condition[1] != "<" && condition[1] != ">")
        {
            return std::vector<std::string>();
        }
        if(is_double)   //������жϽ����ڴ�
        {
            if(!MySQL::is_valid_name(condition[2])) //����ڶ����ǷǷ�����
            {
                return std::vector<std::string>();
            }
            if(condition[0] == condition[2])    //�������Ҫ�жϵ���������ͬ�����Ϸ�
            {
                return std::vector<std::string>();
            }
            return condition;   //����ڵ���������κκ�׺
        }
        if(!is_number(condition[2]))    //����λ�������֣��﷨����
        {
            return std::vector<std::string>();
        }
        //�����ǵ���Ĵ���
        std::vector<std::string> result(condition);
        result[0].append("i");   //�����
        return result;
    }
    else if(condition.size() == 5) //˵���������ַ������ַ����ıȽ�
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
        result[0].append("c");  //�����
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
        if(!MySQL::is_valid_name(condition[0]))    //�����һ���ǷǷ�����
        {
            return std::vector<std::string>();
        }
        if(condition[1] != "=" && condition[1] != "<" && condition[1] != ">")
        {
            return std::vector<std::string>();
        }
        if(MySQL::is_valid_name(condition[2])) //�ܹ�ͨ������ж�˵��������������
        {
            if(condition[0] == condition[2])    //�������Ҫ�жϵ���������ͬ�����Ϸ�
            {
                return std::vector<std::string>();
            }
            return condition;   //�����int���жϱ���������κκ�׺
        }
        else    //ͨ���������˵��������������������int�ı��ʽ���ж�
        {
            if(!is_number(condition[2]))
            {
                error_msg();
                return std::vector<std::string>();
            }
            std::vector<std::string> result(condition);
            result[0].append("i");   //�����
            result.emplace_back("-1");  //���ȴ���3˵����������������
            return result;
        }
    }
    else if(condition.size() == 5) //˵���������ַ������ַ����ıȽϣ�˵��������������������Ӻ�׺
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
        result[0].append("c");  //�����
        result.emplace_back("-1");
        return result;
    }
    else
    {
        return std::vector<std::string>();
    }
}
