#ifndef MYSQL_UTILITY_FUNC_H
#define MYSQL_UTILITY_FUNC_H
#include <iostream>
#include <assert.h>
#include <windows.h>
#include <set>
#include <vector>

void error_msg(const std::string &message = "");

void complete_msg(const std::string &message = "");

void fail_msg(const std::string &message);

void sub_replace(std::string &resource_str, const std::string &sub_str, const std::string &new_str);

bool is_number(const std::string &word);

void print(const std::vector<std::vector<std::string>> &data, bool size = false);

std::string get_time();

template <typename T>
bool compare(T a, T b, const std::string &op)
{
    if (op == ">")
        return a > b;
    else if (op == "<")
        return a < b;
    else
        return a == b;
}

void get_ranks_length(const std::vector<std::vector<std::string>> &vcs, std::vector<int> &length);

void print_row(const std::vector<int> &gap_data);

#endif //MYSQL_UTILITY_FUNC_H
