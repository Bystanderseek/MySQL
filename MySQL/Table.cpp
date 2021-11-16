#include <cassert>
#include <cctype>
#include <utility>
#include <algorithm>
#include <direct.h>
#include <fstream>
#include "Table.h"
#include "utility_func.h"

Table::Table(std::string Table_name, std::vector<std::string> &name,
             std::vector<std::string> &type, std::string Primary, bool read_file) :
                table_name(std::move(Table_name)),
                primary_key(Primary)
{
    if(!primary_key.empty())    //���ȼ�¼������λ��
    {
        for(int i = 0; i < name.size(); ++i)
        {
            if(name[i] == primary_key)
            {
                primary_key_place = i;  //��������������б��е�λ��
                break;
            }
        }
    }
    if(read_file)   //����Ǵ��ļ��ж����
    {
        for(int i = 0; i < name.size(); ++i)
        {
            property_index[name[i]] = i;    //��¼�����±�
            property_vc.emplace_back(std::move(name[i]), std::move(type[i]));
        }
        return;
    }
    assert(name.size() == type.size()); //name �� type�ĳ��Ȳ���ȣ�����������ᷢ����
    assert(mkdir((basic_path + table_name).c_str()) == 0);  //�����ļ������ж������ǵ�ɾȥ
    std::ofstream os(basic_path + table_name + "\\property.txt");
    assert(os);
    if(primary_key.empty())     os << "NULL" << std::endl;   //��д�����������Ϊ�վ�д��NULL
    else    os << primary_key << std::endl;
    for(int i = 0; i < name.size(); ++i)
    {
        os << name[i] << " " << type[i] << std::endl;
        property_index[name[i]] = i;    //��¼�����±�
        property_vc.emplace_back(std::move(name[i]), std::move(type[i]));
    }
    os.close();
}

void Table::insert(std::vector<std::string> &values, bool read_file)
{
    if(read_file)   //���ļ��ж�ȡ
    {
        if(!primary_key.empty())
        {
            std::string key = values[primary_key_place];
            primary_key_table.insert({key, data.size()});
        }
        data.emplace_back(values);
        return;
    }
    if(values.size() != property_vc.size())
    {
        fail_msg("Column doesn't match value");
        return;
    }
    for(int i = 0; i < values.size(); ++i)
    {
        if(!match(values[i], property_vc[i].second))
        {
            fail_msg("Type mismatch");
            return;
        }
    }
    if(!primary_key.empty())
    {
        std::string key = values[primary_key_place];
        if (primary_key_table.find(key) != primary_key_table.end())
        {
            fail_msg("Duplicate primary key");  //�����ظ�
            return;
        }
        primary_key_table.insert({key, data.size()});   //��������
    }
    data.emplace_back(values);
    std::ofstream os(basic_path + table_name + "\\data.txt", std::ofstream::app);
    assert(os);
    for(const auto &r : values)
    {
        os << r << " ";
    }
    os << std::endl;
    os.close();
    complete_msg();
}

bool Table::match(std::string &value, const std::string &type)
{
    if(*type.begin() == 'c')    //�����char
    {
        if(*(--value.end()) != 'c')     return false;
        value.erase(--value.end()); //ȥ��ĩβ�ı��
        std::string num("0123456789");
        int length = stoi(type.substr(type.find_first_of(num)));
        if(value.size() > length)   //�����޶��ĳ�����
            return false;
        else
            return true;
    }
    else
    {
        if(*(--value.end()) != 'i')     return false;
        value.erase(--value.end()); //ȥ��ĩβ�ı��
        return std::all_of(value.cbegin(), value.cend(), isdigit);
    }
}

void Table::desc()
{
    std::vector<std::vector<std::string>> vvcs;
    vvcs.emplace_back(std::vector<std::string>{table_name});
    for(const auto &r : property_vc)
    {
        if(r.first == primary_key)
            vvcs.emplace_back(std::vector<std::string>{r.first + " " + r.second + "(primary key)"});
        else
            vvcs.emplace_back(std::vector<std::string>{r.first + " " + r.second});
    }
    print(vvcs);
}

void Table::delete_all()
{
    int size = data.size();
    data.clear();
    if(!primary_key.empty())    //����������ڣ��Ͱ�������Ҳ���
    {
        primary_key_table.clear();
    }
    write();
    complete_msg(std::to_string(size) + " rows affected");
}

void Table::delete_(const std::vector<std::string> &expression)
{
    std::vector<int> result_single;
    if(expression.size() == 3)  //�����int���ʽ
    {
        //���жϸ������Ƿ����
        std::pair<bool, int> property_result = is_property_exist(expression[0], true);
        if(!property_result.first)
        {
            error_msg("Unknown " + expression[0]);
            return;
        }
        if(expression[1] != "<" && expression[1] != "=" && expression[1] != ">")//�жϷ����Ƿ�Ϸ�
        {
            error_msg();
            return;
        }
        if(!is_number(expression[2]))   //�ж������Ƿ�Ϸ�
        {
            error_msg();
            return;
        }
        if(expression[1] == "=" && expression[0] == primary_key)    //���Ҫ��������������Ȳ���
        {
            if(primary_key_table.find(expression[2]) != primary_key_table.cend()) //�����������ʵ����
            {
                result_single.emplace_back(primary_key_table.at(expression[2]));
            }
        }
        else    //���ִ�еĲ�����������Ȳ���
        {
            if(expression[1] == "=")
            {
                int num = std::stoi(expression[2]);
                for(int i = 0; i < data.size(); ++i)    //��������
                {
                    if(stoi(data[i][property_result.second]) == num)
                    {
                        result_single.emplace_back(i);
                    }
                }
            }
            else if(expression[1] == "<")
            {
                int num = std::stoi(expression[2]);
                for(int i = 0; i < data.size(); ++i)    //��������
                {
                    if(std::stoi(data[i][property_result.second]) < num)    //ת������
                    {
                        result_single.emplace_back(i);
                    }
                }
            }
            else
            {
                int num = std::stoi(expression[2]);
                for(int i = 0; i < data.size(); ++i)    //��������
                {
                    if(std::stoi(data[i][property_result.second]) > num)    //ת������
                    {
                        result_single.emplace_back(i);
                    }
                }
            }
        }
    }
    else if(expression.size() == 5) //�����char���ʽ
    {
        if(expression[1] != "=" || expression[2] != "'" || expression[4] != "'")    //���Ŵ���
        {
            error_msg();
            return;
        }
        std::pair<bool, int> property_result = is_property_exist(expression[0],false);
        if(!property_result.first)    //���Բ�����
        {
            error_msg("Unknown " + expression[0]);
            return;
        }
        if(property_vc[property_result.second].second == "int") //�жϸ������Ƿ�Ϊint
        {
            error_msg("Type mismatch");
            return;
        }
        //��������
        if(expression[0] == primary_key)  //�����ѯ��������
        {
            if(primary_key_table.find(expression[3]) != primary_key_table.cend())
            {
                result_single.emplace_back(primary_key_table.at(expression[3]));
            }
        }
        else
        {
            for(int i = 0; i < data.size(); ++i)
            {
                if(data[i][property_result.second] == expression[3])
                {
                    result_single.emplace_back(i);  //�ǵò���������
                }
            }
        }
    }
    else
    {
        error_msg();
        return;
    }
    //ɾ����������ɾ������
    if(!primary_key.empty())
    {
        for(const auto &r : result_single)
        {
            primary_key_table.erase(data[r][primary_key_place]);
        }
    }
    int size = result_single.size();
    //�������������ɾ��Ԫ�غ���data�����ݻ�λ��
    for(auto iter = result_single.rbegin(); iter != result_single.rend(); ++iter)
    {
        data.erase(data.cbegin() + *iter);
    }
    complete_msg(std::to_string(size) + " rows affected");
    if(size > 0)    //�����ɾ����Ԫ��ʱ�Ż�ִ��д����
        write();
}

std::pair<bool, int> Table::is_property_exist(const std::string &property_name, bool is_int)
{
    auto beg = property_vc.cbegin();
    auto end = property_vc.cend();
    while (beg != end)
    {
        if(beg->first == property_name)
        {
            if(is_int)
            {
                if(beg->second == "int")    //���Ҫ������ int ��������ȴ����int
                {
                    return std::pair<bool, int>(true, beg - property_vc.cbegin());
                }
                else
                    return std::pair<bool, int>(false, -1);
            }
            else
            {
                return std::pair<bool, int>(true, beg - property_vc.cbegin());
            }
        }
        ++beg;
    }
    return std::pair<bool, int>(false, -1);
}

void Table::write()
{
    std::ofstream os(basic_path + table_name + "\\data.txt");
    assert(os);
    for(const auto &r : data)
    {
        for(const auto &j : r)
        {
            os << j << " ";
        }
        os << std::endl;
    }
}

void Table::select_print(const std::vector<std::string> &property_list) const
{
    std::vector<std::vector<std::string>> vvcs;
    if(property_list.size() == 1 && property_list[0] == "*")
    {
        std::vector<std::string> prop;
        for(const auto &r : property_vc)    //����һ�е��������б�
        {
            prop.emplace_back(r.first);
        }
        vvcs.emplace_back(prop);
        for(const auto &r : data)
        {
            vvcs.emplace_back(r);
        }
    }
    else
    {
        for(const auto &r : property_list)
        {
            if(property_index.find(r) == property_index.cend()) //�ж�ĳ�����Ƿ���ʵ����
            {
                fail_msg(" Unknown column " + r);
                return;
            }
        }
        vvcs.emplace_back(property_list);   //����һ����������
        std::vector<std::string> vcs;
        for(const auto &r : data)
        {
            for(const auto &j : property_list)
            {
                vcs.emplace_back(r[property_index.at(j)]);
            }
            vvcs.emplace_back(vcs);
            vcs.clear();    //��ջ�������
        }
    }
    if(data.empty())    //����ǿռ���
    {
        fail_msg("Empty set");
        return;
    }
    print(vvcs);
}

void Table::select_print(const std::vector<std::string> &property_list, const std::vector<int> &index) const
{
    for(const auto r : index)
    {
        assert(r < data.size());
    }
    std::vector<std::vector<std::string>> vvcs;
    if(property_list.size() == 1 && property_list[0] == "*")
    {
        std::vector<std::string> prop;
        for(const auto &r : property_vc)    //����һ�е��������б�
        {
            prop.emplace_back(r.first);
        }
        vvcs.emplace_back(prop);
        for(const auto &r : index)
        {
            vvcs.emplace_back(data[r]);
        }
    }
    else
    {
        for(const auto &r : property_list)
        {
            if(property_index.find(r) == property_index.cend()) //�ж�ĳ�����Ƿ���ʵ����
            {
                fail_msg(" Unknown column " + r);
                return;
            }
        }
        vvcs.emplace_back(property_list);   //����һ����������
        std::vector<std::string> vcs;
        for(const auto &r : index)
        {
            for(const auto &j : property_list)
            {
                vcs.emplace_back(data[r][property_index.at(j)]);
            }
            vvcs.emplace_back(vcs);
            vcs.clear();    //��ջ�������
        }
    }
    print(vvcs);
}

std::vector<int> Table::select(const Table &table, const std::vector<std::string> &condition)
{
    char type = *(--condition[0].cend());   //�õ���Ԫ�ص����ͱ��
    std::string value = condition[0];
    value.erase(--value.end());   //ɾ���������ͱ��
    if(table.property_index.find(value) != table.property_index.cend()) //�жϱ����Ƿ���ڸ�����
    {
        int index = table.property_index.at(value);     //�õ������������������е��±�
        std::string property_type = table.property_vc[index].second;    //�õ������Ե�����
        if(type == property_type[0])    //���Ե����ͻ���Ϊint������Ϊchar����
        {
            std::vector<int> result;
            if(type == 'c')
            {
                if(condition[1] != "=") //�ַ�����ֻ���ж����
                {
                    error_msg("Mismatch " + condition[1]);
                    return std::vector<int>{-1};
                }
                for(int i = 0; i < table.data.size(); ++i)  //����Ȼ���ж����
                {
                    if(table.data[i][index] == condition[2])
                    {
                        result.emplace_back(i);
                    }
                }
            }
            else
            {
                std::string op = condition[1];
                int num = std::stoi(condition[2]);
                for(int i = 0; i < table.data.size(); ++i)
                {
                    if(compare(std::stoi(table.data[i][index]), num, op))
                    {
                        result.emplace_back(i);
                    }
                }
            }
            return result;
        }
        else
        {
            error_msg("Type mismatch");
            return std::vector<int>{-1};
        }
    }
    error_msg("Unknown column " + value);
    return std::vector<int>{-1};
}

std::vector<std::pair<int, int>> Table::select(const Table &table_1, const Table &table_2,
                                               const std::vector<std::string> &condition)
{
    std::pair<bool, std::string> prop_1 = table_1.match(condition[0]);
    std::pair<bool, std::string> prop_2 = table_2.match(condition[2]);
    if(!prop_1.first || !prop_2.first)  //�������һ��������Բ�ƥ��
    {
        fail_msg("Property mismatch");
        return std::vector<std::pair<int, int>>{{-1, -1}};
    }
    if(prop_1.second != prop_2.second)
    {
        fail_msg("Type mismatch");
        return std::vector<std::pair<int, int>>{{-1, -1}};
    }
    int index_1 = table_1.property_index.at(condition[0]);  //��1�и����Ե��±�
    int index_2 = table_2.property_index.at(condition[2]);
    std::vector<std::pair<int, int>> result;
    if(prop_1.second == "cha")  //�ж�char
    {
        if(condition[1] != "=") //��������ж���ȵĲ������ͷǷ�
        {
            error_msg();
            return std::vector<std::pair<int, int>>{{-1, -1}};
        }
        for(int i = 0; i < table_1.data.size(); ++i)
        {
            for(int j = 0; j < table_2.data.size(); ++j)
            {
                if(table_1.data[i][index_1] == table_2.data[j][index_2])
                {
                    result.emplace_back(i, j);
                }
            }
        }
    }
    else    //�ж�int
    {
        std::string op = condition[1];
        for(int i = 0; i < table_1.data.size(); ++i)
        {
            for(int j = 0; j < table_2.data.size(); ++j)
            {
                if(compare(std::stoi(table_1.data[i][index_1]), std::stoi(table_2.data[j][index_2]), op))
                {
                    result.emplace_back(i, j);
                }
            }
        }
    }
    return result;
}

std::pair<bool, std::string> Table::match(const std::string &property_name) const
{
    if(property_index.find(property_name) != property_index.cend())
    {
        int index = property_index.at(property_name);
        return std::make_pair(true, property_vc[index].second.substr(0, 3));
        //��ȡint����char��ǰ3����ĸ�������Ƚ�
    }
    return std::pair<bool, std::string>(false, "");
}

bool Table::match_with_type(const std::string &property_name) const
{
    std::string name(property_name);
    char type = *(--name.cend());  //ȡĩβ��ǵ�Ԫ������
    name.erase(--name.cend());
    std::pair<bool, std::string> result = match(name);
    if(!result.first)
    {
        return false;
    }
    if(*result.second.cbegin() != type) //ȡ���͵�����ĸ��char ���� int��
    {
        return false;
    }
    return true;
}

void Table::select_print(const std::vector<std::string> &property_list, const Table &table_1, const Table &table_2,
                         const std::vector<std::pair<int, int>> &index)
{
    std::vector<std::vector<std::string>> vvcs;
    if(property_list.size() == 1 && property_list[0] == "*")
    {
        std::vector<std::string> property_set;    //���������Եļ���
        for(const auto &r : table_1.property_vc)
        {
            property_set.emplace_back(r.first);
        }
        for(const auto &r : table_2.property_vc)
        {
            property_set.emplace_back(r.first);
        }
        //�������
        vvcs.emplace_back(property_set);
        //���Ԫ��
        for(const auto &r : index)
        {
            std::vector<std::string> data_set(table_1.data[r.first]);
            for(const auto &j : table_2.data[r.second]) //������2���ѱ�2����Ϣ�����ȥ
            {
                data_set.emplace_back(j);
            }
            vvcs.emplace_back(data_set);
        }
    }
    else
    {
        std::set<std::string> judge_property_list;  //�ж������б����Ƿ����ظ�Ԫ��
        judge_property_list.insert(property_list.cbegin(), property_list.cend());
        if(judge_property_list.size() != property_list.size())
        {
            error_msg("Duplicate property");
            return;
        }
        std::vector<std::string> prop_1;
        std::vector<std::string> prop_2;
        for(const auto &r : property_list)  //�������б��е�Ԫ�ؼ��뵽��Ӧ�ı���
        {
            if(table_1.match(r).first)
            {
                prop_1.emplace_back(r);
            }
            if(table_2.match(r).first)
            {
                prop_2.emplace_back(r);
            }
            if(table_1.match(r).first && table_2.match(r).first) //���������ͬʱ�����������У��ͱ���
            {
                fail_msg("Ambiguity " + r);
                return;
            }
        }
        vvcs.emplace_back(property_list);   //�Ȱ�������ӵ�����б��������
        for(const auto &r : index)
        {
            std::vector<std::string> data_set;
            for(const auto &j : prop_1)
            {
                int index_1 = table_1.property_index.at(j);
                data_set.emplace_back(table_1.data.at(r.first)[index_1]);
            }
            for(const auto &j : prop_2)
            {
                int index_2 = table_2.property_index.at(j);
                data_set.emplace_back(table_2.data.at(r.second)[index_2]);
            }
            vvcs.emplace_back(data_set);
        }
    }
    print(vvcs);
}

std::vector<std::pair<int, int>> Table::select(const Table &table_1, const Table &table_2,
                                               std::vector<std::string> &condition_1,
                                               std::vector<std::string> &condition_2)
{
    bool is_connect_condition_1 = false;
    bool is_connect_condition_2 = false;
    if(is_connect_condition(table_1, table_2, condition_1)) //�ж��ǲ�����������
    {
        std::pair<bool, std::string> prop_1 = table_1.match(condition_1[0]);
        std::pair<bool, std::string> prop_2 = table_2.match(condition_1[2]);
        if(prop_1.second != prop_2.second)
        {
            fail_msg("Type mismatch");
            return std::vector<std::pair<int, int>>{{-1, -1}};
        }
        is_connect_condition_1 = true;
    }
    if(is_connect_condition(table_1, table_2, condition_2)) //�ж��ǲ�����������
    {
        std::pair<bool, std::string> prop_1 = table_1.match(condition_1[0]);
        std::pair<bool, std::string> prop_2 = table_2.match(condition_1[2]);
        if(prop_1.second != prop_2.second)
        {
            fail_msg("Type mismatch");
            return std::vector<std::pair<int, int>>{{-1, -1}};
        }
        is_connect_condition_2 = true;
    }
    if(!is_connect_condition_1 && !is_connect_condition_2)  //���1��2��������������
    {
        fail_msg("Missing connection condition");
        return std::vector<std::pair<int, int>>{{-1, -1}};
    }
    if(is_connect_condition_1 && is_connect_condition_2)    //1��2������������
    {
        std::vector<std::pair<int, int>> index_1 = Table::select(table_1, table_2, condition_1);
        std::vector<std::pair<int, int>> index_2 = Table::select(table_1, table_2, condition_2);
        if((index_1.size() == 1 && index_1[0].first == -1) || (index_2.size() == 1  && index_2[0].first == -1)) //˵�����˴���
        {
            return std::vector<std::pair<int, int>>{{-1, -1}}; //��������-1
        }
        std::vector<std::pair<int, int>> result;
        for(const auto &r : index_1)
        {
            for(const auto &j : index_2)
            {
                if(r == j)
                {
                    result.emplace_back(r);
                }
            }
        }
        return result;
    }
    else if(is_connect_condition_1 && !is_connect_condition_2)  //1������������2����
    {
        condition_2.resize(3);  //�ѱ��λ���ӵ�
        std::vector<int> index_2;
        int tag;    //������ĸ���ƥ��������
        if(table_2.match_with_type(condition_2[0])) //�ж����ĸ��������
        {
            index_2 = select(table_2, condition_2);
            tag = 2;
        }
        else if(table_1.match_with_type(condition_2[0]))
        {
            index_2 = select(table_1, condition_2);
            tag = 1;
        }
        else
        {
            error_msg();
            return std::vector<std::pair<int, int>>{{-1, -1}};
        }
        std::vector<std::pair<int, int>> index_1 = Table::select(table_1, table_2, condition_1);
        if((index_2.size() == 1 && index_2[0] == -1) || (index_1.size() == 1  && index_1[0].first == -1)) //˵�����˴���
        {
            return std::vector<std::pair<int, int>>{{-1, -1}}; //��������-1
        }
        std::vector<std::pair<int, int>> result;
        for(const auto &r : index_2)
        {
            for(const auto &j : index_1)
            {
                if(tag == 1 && r == j.first)   //first�ǵ�һ������ɸѡ�������ݣ�second�ǵڶ������е�ɸѡ��������
                {
                    result.emplace_back(j);
                }
                else if(tag == 2 && r == j.second)
                {
                    result.emplace_back(j);
                }
            }
        }
        return result;
    }
    else    //2������������1����
    {
        condition_1.resize(3);
        std::vector<int> index_1;
        int tag;
        if(table_2.match_with_type(condition_1[0])) //�ж����ĸ��������
        {
            index_1 = select(table_2, condition_1);
            tag = 2;
        }
        else if(table_1.match_with_type(condition_1[0]))
        {
            index_1 = select(table_1, condition_1);
            tag = 1;
        }
        else
        {
            error_msg();
            return std::vector<std::pair<int, int>>{{-1, -1}};
        }
        std::vector<std::pair<int, int>> index_2 = Table::select(table_1, table_2, condition_2);
        if((index_1.size() == 1 && index_1[0] == -1) || (index_2.size() == 1  && index_2[0].first == -1)) //˵�����˴���
        {
            return std::vector<std::pair<int, int>>{{-1, -1}}; //��������-1
        }
        std::vector<std::pair<int, int>> result;
        for(const auto &r : index_1)
        {
            for(const auto &j : index_2)
            {
                if(tag == 1 && r == j.first)   //first�ǵ�һ������ɸѡ�������ݣ�second�ǵڶ������е�ɸѡ��������
                {
                    result.emplace_back(j);
                }
                else if(tag == 2 && r == j.second)
                {
                    result.emplace_back(j);
                }
            }
        }
        return result;
    }
}

bool Table::is_connect_condition(const Table &table_1, const Table &table_2,
                                 const std::vector<std::string> &condition)
{
    if(condition.size() != 3)   //���������Ĵ�Сһ��Ϊ3
        return false;
    //���ĳ��Ԫ��ͬʱƥ�����������ͱ���
    if((table_1.match(condition[0]).first && table_1.match(condition[2]).first) ||
        table_2.match(condition[0]).first && table_2.match(condition[2]).first)
    {
        return false;
    }
    if(table_1.match(condition[0]).first && table_2.match(condition[2]).first)
    {
        return true;
    }
    return false;
}
