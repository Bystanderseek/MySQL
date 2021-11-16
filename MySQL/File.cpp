#include <direct.h>
#include "File.h"


void delete_directory_all(const std::string &name)
{
    std::string path(basic_path + name);
    remove((path + "\\data.txt").c_str());
    remove((path + "\\primarykey.txt").c_str());
    remove((path + "\\property.txt").c_str());
}
