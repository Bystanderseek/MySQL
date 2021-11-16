#include "MySQL.h"
#include "utility_func.h"
#include <stack>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cassert>

const std::set<std::string>& MySQL::illegal_name()
{
    static std::set<std::string> illegal{"table", "create", "delete", "select", "drop",
                                         "quit", "from", "int", "char", "and", "primarykey"};
    return illegal;
}

Database MySQL::database;

MySQL::MySQL()
{
    sql_operator = {
            {"create", create},
            {"delete", delete_},
            {"select", select},
            {"drop", drop},
            {"quit", quit},
            {"insert", insert},
            {"desc", desc},
            {"show", show},
    };
}

std::pair<bool, std::string> MySQL::grammar_analyze(std::string &line)
{
    std::string::iterator iter = line.end();
    --iter;
    if(*iter != ';')    //不以分号结尾
    {
        return std::pair<bool, std::string>{false, "Missing ';' at end of line"};
    }
    *iter = ' ';        //转为空格
    std::stack<char> bracketStack;
    for(const auto &r : line)
    {
        if(r == '(')    bracketStack.push('(');
        if(r == ')')
        {
            if(bracketStack.empty())
                return std::pair<bool, std::string>{false, "Bracket mismatch"};
            else    bracketStack.pop();
        }
    }
    if(!bracketStack.empty())
        return std::pair<bool, std::string>{false, "Bracket mismatch"};  //栈不空，说明括号不匹配
    if(std::count(line.cbegin(), line.cend(), '\'') % 2 != 0)   //如果单引号不匹配
        return std::pair<bool, std::string>{false, "single quotes mismatch"};
    std::for_each(line.begin(), line.end(), [](char &c) {c = std::tolower(c);}); //将字符转为小写
    sub_replace(line, "(", " ( ");
    sub_replace(line, ")", " ) ");
    sub_replace(line, ",", " , ");
    sub_replace(line, "'", " ' ");  //如果单引号匹配出了问题，记得来检查下
    return std::pair<bool, std::string>{true, line};
}

bool MySQL::is_valid_name(const std::string &name)
{
    if(std::find(illegal_name().cbegin(), illegal_name().cend(), name) != illegal_name().cend())
        return false;   //如果起的名称是非法名称
    if(std::isalpha(name.at(0)) || name.at(0) == '_')
    {
        for(const auto r : name)
        {
            if(!std::isalnum(r) && r != '_')    return false;
        }
        return true;
    }
    else    return false;
}

void MySQL::execute(const std::pair<bool, std::string> &line) const
{
    if(!line.first)
    {
        error_msg(line.second);
        return;
    }
    std::stringstream ss(line.second);
    std::string first_word;
    ss >> first_word;
    if(sql_operator.find(first_word) != sql_operator.end()) //如果指令存在
    {
        sql_operator.at(first_word)(line.second);   //const成员函数不能修改内部的东西，故用at
    }
    else
    {
        std::cout << "Misunderstand \"" << first_word << "\"" << std::endl;
        return;
    }
}

void MySQL::program_interface(std::string &line) const
{
    std::pair<bool, std::string> result = grammar_analyze(line);
    execute(result);
}

std::vector<std::string>& MySQL::get_vec(const std::string &line)
{
    static std::vector<std::string> vstr;
    vstr.clear();
    std::stringstream ss(line);
    std::string word;
    while (ss >> word)
    {
        vstr.emplace_back(word);
    }
    return vstr;
}

void MySQL::create(const std::string &line)
{
    std::vector<std::string> &sentence = get_vec(line);
    if(sentence.size() < 7)     //如果语句长度短于7，就说明其中没有属性。
    {
        error_msg();
        return;
    }
    if(sentence[1] == "table" && is_valid_name(sentence[2]))
    {
        if((sentence[3] != "(") || (*sentence.rbegin() != ")")) //判断首尾括号匹配关系
        {
            error_msg("Bracket mismatch");
            return;
        }
        sentence.erase(sentence.begin() + 3);   //删除头尾的左右括号
        sentence.erase((++sentence.rbegin()).base());
        std::string table_name = sentence[2];
        std::string primary_key;
        auto primary_iter = std::find(sentence.cbegin(), sentence.cend(), "primarykey");
        if(primary_iter != sentence.cend())         //如果主键存在
        {
            if(sentence.cend() - primary_iter != 4)
            {
                error_msg("The primary key is not in the right place");
                return;
            }
            if(*(primary_iter + 1) == "(" && is_valid_name(*(primary_iter + 2)) &&
               *(primary_iter + 3) == ")")
            {
                primary_key = *(primary_iter + 2);  //记录主键
                sentence.erase(primary_iter, sentence.cend());  //把主键的信息删除，便于后续处理
            }
            else
            {
                error_msg("The primary key is not in the right place");
                return;
            }
        }
        int initial_size = 5;
        std::vector<std::string> name;
        std::vector<std::string> type;
        std::set<std::string> name_record;  //记录名称是否重复的集合，为了降低时间复杂度
        name.reserve(initial_size);
        type.reserve(initial_size);
        auto size = sentence.size();
        enum read{
            read_name,
            read_type,
            read_symbol,
        } state = read_name;  //当前正在读的状态
        for(int i = 3; i < size; ++i)   //从表名的下一位开始
        {
            if(state == read_name)       //读属性名
            {
                if(is_valid_name(sentence[i]))  //如果名称合法
                {
                    if(name_record.insert(sentence[i]).second)  //如果名称插入成功，就说明名字中没有重复的
                    {
                        name.emplace_back(sentence[i]);
                        state = read_type;
                    }
                    else
                    {
                        error_msg("Duplicate column name \"" + sentence[i] + "\"");
                        return;
                    }
                }
                else
                {
                    error_msg("Invalid name " + sentence[i]);
                    return;
                }
            }
            else if(state == read_type)     //读类型名
            {
                if(sentence[i] == "char")
                {
                    if(i + 3 >= size)   //越界检查
                    {
                        error_msg();
                        return;
                    }
                    if(sentence[i + 1] == "(" && is_number(sentence[i + 2]) && sentence[i + 3] == ")")
                    {
                        std::string char_type("char");
                        for(int j = 1; j < 4; ++j)
                        {
                            char_type.append(sentence[i + j]);
                        }
                        type.emplace_back(char_type);
                        i += 3;
                        state = read_symbol;
                    }
                    else
                    {
                        error_msg("unkown " + sentence[i]);
                        return;
                    }
                }   //是char类型
                else if(sentence[i] == "int")
                {
                    type.emplace_back(sentence[i]);
                    state = read_symbol;
                }
                else
                {
                    error_msg("Invalid type");
                    return;
                }
            }
            else
            {
                if(sentence[i] == ",")
                {
                    state = read_name;
                    continue;
                }
                else
                {
                    error_msg();
                    return;
                }
            }
        }
        if(state != read_name)
        {
            error_msg();
            return;
        }
        //如果提供了主键但是主键并不存在于类型名中
        if(!primary_key.empty() && name_record.find(primary_key) == name_record.cend())
        {
            error_msg("Primary key doesn't exist");
            return;
        }
        database.create_table(table_name, name, type, primary_key);
    }
    else
    {
        error_msg();
        return;
    }
}

void MySQL::insert(const std::string &line)
{
    std::vector<std::string> &sentence = get_vec(line);
    if(sentence.size() < 6)     //说明其中没有东西
    {
        error_msg();
        return;
    }
    else
    {
        if(sentence[1] == "into" && is_valid_name(sentence[2]) && sentence[3] == "values"
        && sentence[4] == "(" && *(--sentence.end()) == ")")
        {
            std::string table_name = sentence[2];
            sentence.erase(sentence.begin(), sentence.begin() + 5); //删去多余的语句和括号
            sentence.erase((--sentence.end()));
            enum read{
                read_value,
                read_symbol,
            } state = read_value;  //当前正在读的状态
            std::vector<std::string> values;
            values.reserve(5);
            auto size = sentence.size();
            for(int i = 0; i < size; ++i)
            {
                if(state == read_value)
                {
                    if(is_number(sentence[i]))
                    {
                        sentence[i].push_back('i');     //做一个标记
                        values.emplace_back(sentence[i]);
                    }
                    else if(sentence[i] == "'")
                    {
                        if(i + 2 >= size)
                        {
                            error_msg();
                            return;
                        }
                        if(sentence[i + 2] == "'")
                        {
                            sentence[i + 1].push_back('c');     //做一个标记
                            values.emplace_back(sentence[i + 1]);
                        }
                        i += 2;
                    }
                    else
                    {
                        error_msg();
                        return;
                    }
                    state = read_symbol;
                }
                else
                {
                    if(sentence[i] == ",")
                    {
                        state = read_value;
                    }
                    else
                    {

                        error_msg();
                        return;
                    }
                }
            }
            database.insert(table_name, values);
        }
        else
        {
            error_msg();
            return;
        }
    }
}

void MySQL::select(const std::string &line)
{
    std::vector<std::string> &sentence = get_vec(line);
    auto iterator_from = std::find(sentence.begin(), sentence.end(), "from");
    if(iterator_from == sentence.cend())    //无from语句
    {
        error_msg();
        return;
    }
    std::vector<std::string> target((++sentence.begin()), iterator_from);
    //如果目标数组为空或者没有关系数组（table),注意此步让iterator_from后移了一位
    if(target.empty() || (++iterator_from) == sentence.cend())
    {
        error_msg();
        return;
    }
    auto iterator_where = std::find(iterator_from, sentence.end(), "where");//应停留在where或者cend
    std::vector<std::string> relation(iterator_from, iterator_where);//从from的后一位开始直到where或者cend
    if(relation.empty())    //判断是否有关系表
    {
        error_msg();
        return;
    }
    std::vector<std::string> target_property;   //保存最终要查询的目标属性的数组
    if(target.size() % 2 != 0)  //目标表达式一定是奇数
    {
        if(target.size() == 1 && target[0] == "*")  //搜索全部
        {
            target_property.emplace_back("*");
        }
        else
        {
            for(int i = 0; i < target.size(); ++i)
            {
                if(i % 2 == 0)  //下标的偶数位必须为字符串，奇数位必须为逗号
                {
                    if(is_valid_name(target[i]))    //判断是否是有效的名字
                    {
                        target_property.emplace_back(target[i]);
                    }
                    else
                    {
                        error_msg();
                        return;
                    }
                }
                else
                {
                    if(target[i] != ",")    //如果不为逗号，说明出错了
                    {
                        error_msg();
                        return;
                    }
                }
            }
        }
        //调用函数
    }
    else
    {
        error_msg();
        return;
    }
    //判定关系数组，表最多为2
    if(relation.size() == 1 && is_valid_name(relation[0]))  //只有一个关系
    {
        //什么都不需要做，说明只有一个表并且表符合语法规则
    }
    else if(relation.size() == 3 && is_valid_name(relation[0]) && is_valid_name(relation[2])
            && relation[1] == ",")      //有两个关系
    {
        relation.erase(relation.begin() + 1);   //把逗号删去
    }
    else
    {
        error_msg();
        return;
    }
    //判断有没有选择条件
    if(iterator_where == sentence.cend())   //如果没有选择条件，此时迭代器在cend
    {
        database.select(target_property, relation);
    }
    else    //如果有选择条件
    {
        std::vector<std::string> condition(++iterator_where, sentence.end());//从where的下一个位置开始
        if(condition.empty())   //说明where是最后一个元素，语法错误
        {
            error_msg();
            return;
        }
        database.select(target_property, relation, condition);
    }
}

void MySQL::delete_(const std::string &line)
{
    std::vector<std::string> &sentence = get_vec(line);
    if(sentence.size() < 3)
    {
        error_msg();
        return;
    }
    if(sentence[1] == "from" && is_valid_name(sentence[2]))
    {
        if(sentence.size() == 3)
        {
            database.delete_table_all(sentence[2]);
        }
        else if(sentence.size() >= 7 && sentence[3] == "where")
        {
            std::vector<std::string> vcs(sentence.cbegin() + 4, sentence.cend());
            database.delete_(sentence[2], vcs);
        }
        else
        {
            error_msg();
            return;
        }
    }
    else
    {
        error_msg();
        return;
    }
}

void MySQL::drop(const std::string &line)
{
    std::vector<std::string> &sentence = get_vec(line);
    if(sentence.size() == 3)
    {
        if(sentence[1] == "table" && is_valid_name(sentence[2]))
        {
            database.drop_table(sentence[2]);
        }
        else
        {
            error_msg();
            return;
        }
    }
    else
    {
        error_msg();
        return;
    }
}

void MySQL::quit(const std::string &line)
{
    if(line == "quit ")     exit(0);
    else    error_msg();
}

void MySQL::desc(const std::string &line)
{
    std::vector<std::string> &vcs = get_vec(line);
    if(vcs.size() != 2)
    {
        error_msg();
        return;
    }
    database.desc(vcs[1]);
}

void MySQL::show(const std::string &line)
{
    std::vector<std::string> &vcs = get_vec(line);
    if(vcs.size() != 2 || vcs[1] != "tables")
    {
        error_msg();
        return;
    }
    database.show_tables();
}


void MySQL::login_show()
{
    std::string user("admin");
    std::cout << user << ", Welcome to the MySQL monitor.  Commands end with ;" << std::endl;
    std::cout << "Your MySQL connection id is 18\n"
                 "Server version: 8.0.19 MySQL Community Server - GPL" << std::endl;
    std::cout << "Landing time: " << get_time() << std::endl;
    std::cout << std::endl;
}
