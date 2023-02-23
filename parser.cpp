#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>

#include "util.hpp"

const std::string src_path = "data/input";					  // Boost库HTML文件的存放路径
const std::string output = "data/original_html/original.txt"; // 去标签化后文件的存放路径

typedef struct DocInfo
{
	std::string title;	 // 网页标题
	std::string content; // 文档内容
	std::string url;	 // 文档在官网的url

} DocInfo_t;

bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list);
bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *result);
bool SaveHtml(const std::vector<DocInfo_t> &result, const std::string &output);

static bool ParseTitle(const std::string &file, std::string *title)
{
	std::size_t begin = file.find("<title>");

	if (begin == std::string::npos)
		return false;
	std::size_t end = file.find("</title>");
	if (end == std::string::npos)
		return false;

	begin += std::string("<title>").size();

	if (begin > end) // 确保begin end 位置合适
	{
		return false;
	}
	*title = file.substr(begin, end - begin);

	return true;
}

static bool ParseContent(const std::string &file, std::string *content)
{
	// 去标签, 基于一个简易的状态机
	enum status
	{
		LABLE,
		CONTENT
	};
	enum status s = LABLE;

	// 在遍历时, 只要碰到了>就意味着当前标签被处理完毕
	// 只要碰到了<意味着新的标签开始了
	for (char c : file)
	{
		switch (s)
		{
		case LABLE:
			if (c == '>')
				s = CONTENT;
			break;
		case CONTENT:
			if (c == '<')
				s = LABLE;
			else
			{
				// 我们不想保留原始文件中的\n, 我们需要\n作为html解析之后文本的分隔符
				if (c == '\n')
					c = ' ';
				content->push_back(c);
			}
			break;
		default:
			break;
		}
	}

	return true;
}

static bool ParseUrl(const std::string &file_path, std::string *url)
{
	std::string url_head = "https://www.boost.org/doc/libs/1_81_0/doc/html";
	std::string url_tail = file_path.substr(src_path.size());

	*url = url_head + url_tail;
	return true;
}

bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list)
{
	namespace fs = boost::filesystem;
	fs::path root_path(src_path);

	// 判断路径是否存在
	if (!fs::exists(root_path))
	{
		std::cerr << src_path << " not exists" << std::endl;
		return false;
	}

	// 定义一个空的迭代器, 用来进行判断递归结束
	fs::recursive_directory_iterator end;
	for (fs::recursive_directory_iterator it(root_path); it != end; it++)
	{
		// 满足两个条件: 1.文件是普通文件 2. 文件后缀为html
		if (!fs::is_regular_file(*it))
			continue;
		if (it->path().extension() != ".html")
			continue;
		// std::cout << "debug: " << it->path().string() << std::endl;
		files_list->push_back(it->path().string());
	}

	return true;
}

void ShowDoc(const DocInfo_t &doc)
{
	std::cout << "debug info:" << std::endl;
	std::cout << "title: " << doc.title << std::endl;
	std::cout << "content: " << doc.content << std::endl;
	std::cout << "url: " << doc.url << std::endl;
}

bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results)
{
	for (const std::string &file : files_list)
	{
		// 1. 读取文件
		std::string result;
		if (!util::FileUtil::ReadFile(file, &result))
		{
			continue;
		}

		// 2. 解析指定文件, 提取title
		DocInfo_t doc;
		if (!ParseTitle(result, &doc.title))
		{
			continue;
		}

		// 3. 解析指定的文件, 提取(去标签)content
		if (!ParseContent(result, &doc.content))
		{
			continue;
		}
		// 4. 解析指定的文件路径, 构建url
		if (!ParseUrl(file, &doc.url))
		{
			continue;
		}

		// 程序执行到这里一定是完成了解析任务, 当前文档的结果都保存到了doc里面
		//  本质会发生拷贝,效率较低
		results->push_back(std::move(doc));
		// ShowDoc(doc);
	}

	return true;
}

bool SaveHtml(const std::vector<DocInfo_t> &result, const std::string &output)
{
#define SEP '\3'
	std::ofstream out(output, std::ios::out | std::ios::binary);
	if (!out.is_open())
	{
		std::cerr << "open " << output << "failed!" << std::endl;
		return false;
	}

	// 进行内容的写入了
	for (auto &it : result)
	{
		std::string out_string;
		out_string = it.title;
		out_string += SEP;
		out_string += it.content;
		out_string += SEP;
		out_string += it.url;
		out_string += '\n';

		out.write(out_string.c_str(), out_string.size());
	}

	out.close();
	return true;
}

int main()
{
	std::vector<std::string> files_list;

	// 1.把Boost库每个html文件(包含路径),保存到files_list中,方便后期对每个文件的读取
	if (!EnumFile(src_path, &files_list))
	{
		std::cerr << "enum file name error!" << std::endl;
		return 1;
	}

	// 2.按照files_list读取每个文件的内容, 并进行分析处理
	std::vector<DocInfo_t> results;
	if (!ParseHtml(files_list, &results))
	{
		std::cerr << "parse html error" << std::endl;
		return 2;
	}

	// 3.把所有解析完毕的文件内容,写入到original.txt文件中, 按照\3作为每个文档的分隔符
	if (!SaveHtml(results, output))
	{
		std::cerr << "save html error" << std::endl;
		return 3;
	}

	return 0;
}
