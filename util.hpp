#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>


namespace util
{
    class FileUtil
    {
    public:
        static bool ReadFile(const std::string &file_path, std::string *out)
        {
            std::ifstream in(file_path, std::ios::in);
            if (!in.is_open())
            {
                std::cerr << "open file " << file_path << "error" << std::endl;
                return false;
            }
            // 按行将文件内容读取到out中
            std::string line;
            while (std::getline(in, line))
            {
                *out += line;
            }

            in.close();
            return true;
        }
    };

    class StringUtil
    {
        public:
            static void CutString(const std::string &target, std::vector<std::string> *out, std::string sep)
            {
                // Boost split 切分字符串
                boost::split(*out, target, boost::is_any_of(sep), boost::token_compress_on);
            }   
    };
}
