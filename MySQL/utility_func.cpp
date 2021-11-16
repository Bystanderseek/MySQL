#include "utility_func.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

void error_msg(const std::string& message)
{
    std::cin.clear();
    std::cin.sync();
    std::cout << "You have an error in your SQL syntax. " << message << std::endl;
}

void complete_msg(const std::string& message)
{
    std::cout << "Query OK. " << message << std::endl;
}

void fail_msg(const std::string& message)
{
    std::cin.clear();
    std::cin.sync();
    std::cout << message << std::endl;
}

void sub_replace(std::string &resource_str, const std::string &sub_str, const std::string &new_str)
{
    std::string::size_type pos = 0;
    while((pos = resource_str.find(sub_str, pos)) != std::string::npos)   //替换所有指定子串
    {
        resource_str.replace(pos, sub_str.length(), new_str);
        pos += new_str.length();
    }
}

std::string get_time()
{
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    std::string time;
    time.append(std::to_string(sys.wYear) + "/");
    time.append(std::to_string(sys.wMonth) + "/");
    time.append(std::to_string(sys.wDay) + " ");
    time.append(std::to_string(sys.wHour) + ":");
    time.append(std::to_string(sys.wMinute) + ":");
    time.append(std::to_string(sys.wSecond));
    return time;
}

bool is_number(const std::string &word)
{
    return std::all_of(word.cbegin(), word.cend(), isdigit);
}

void get_ranks_length(const std::vector<std::vector<std::string>> &vcs, std::vector<int> &length)
{
    int row = vcs.size();
    int list = vcs[0].size();
    for(size_t i = 0; i < list; ++i)
    {
        int max = 0;
        for(size_t j = 0; j < row; ++j)
        {
            if(vcs[j][i].size() > max)
            {
                max = vcs[j][i].size();
            }
        }
        length.emplace_back(max);
    }
}

void print_row(const std::vector<int> &gap_data)
{
    std::vector<int> vci;
    vci.emplace_back(0);
    vci.insert(vci.cend(), gap_data.cbegin(), gap_data.cend());
    for(int i = 1; i < vci.size(); ++i)
    {
        vci[i] += 2;
    }
    int i = 0, count = 0, gap;
    while (i < vci.size())
    {
        gap = vci[i];
        if(count == gap)
        {
            std::cout << "+";
            ++i;        //移动到下个位置
            count = 0;  //刷新计数器
        }
        else
        {
            std::cout << "-";
            ++count;
        }
    }
    std::cout << std::endl;
}

void print(const std::vector<std::vector<std::string>> &data, bool size)
{
    std::vector<int> gap_data;
    get_ranks_length(data, gap_data);
    int row = data.size();
    int list = data[0].size();
    print_row(gap_data);
    for(int i = 0; i < row; ++i)
    {
        if(i == 1)
            print_row(gap_data);
        for(int j = 0; j < list; ++j)
        {
            std::cout << "| " << std::left << std::setw(gap_data[j] + 1) << data[i][j];
        }
        std::cout << "|" << std::endl;
    }
    print_row(gap_data);
    if(size)
    {
        complete_msg(std::to_string(data.size() - 1) + " row affected");
    }
}
