#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "util.hpp"

namespace Index
{
	struct DocInfo
	{
		std::string title;	 // 文档标题
		std::string content; // 文档对应去标签之后的内容
		std::string url;	 // 官网文档的url
		uint64_t doc_id;	 // 文档ID, 暂时不做过多理解
	};

	struct InvertedElem
	{
		uint64_t doc_id; //
		std::string word;
		int weight;
	};

	typedef std::vector<InvertedElem> InvertedList;

	class Index
	{
	private:
		// 正排索引的数据结构我们用数组, 数组的下标作为文档ID,
		std::vector<DocInfo> forward_index; // 正排索引
		// 倒排索引一个是一个关键字和一组InvertedElem对应[关键字和倒排拉链的映射关系]
		std::unordered_map<std::string, InvertedList> inverted_index;

	public:
		Index() {}
		~Index() {}

	public:
		// 根据doc_id找到文档内容
		DocInfo *GetForwardIndex(uint64_t doc_id)
		{
			if (doc_id >= forward_index.size())
			{
				std::cerr << "error: doc_id out range!" << std::endl;
				return nullptr;
			}

			return &forward_index[doc_id];
		}

		// 根据关键字string获得倒排拉链
		InvertedList *GetInvertedList(const std::string &word)
		{
			auto it = inverted_index.find(word);
			if (it == inverted_index.end())
			{
				std::cerr << "error: not found word" << std::endl;
				return nullptr;
			}
			return &it->second;
		}

		// 根据去标签,格式化之后的文档, 构建正排和倒排索引
		bool BuildIndex(const std::string &input) // parse处理完毕的数据交给我
		{
			std::ifstream in(input, std::ios::in | std::ios::binary);
			if (!in.is_open())
			{
				std::cerr << "error: " << input << "open failed" << std::endl;
				return false;
			}
			std::string line;
			while (std::getline(in, line))
			{
				DocInfo *doc = BuildForWardIndex(line);
				if (doc == nullptr)
				{
					std::cerr << "error: build" << line << " failed!" << std::endl;
					continue;
				}

				BuildInvertedIndex(*doc);
			}

			return true;
		}

	private:
		DocInfo *BuildForWardIndex(const std::string &line)
		{
			// 1.解析line进行字符串切分
			// line -> string × 3 => title content url
			std::vector<std::string> results;
			const std::string sep = "\3"; // 分隔符
			util::StringUtil::CutString(line, &results, sep);
			if (results.size() != 3)
			{
				return nullptr;
			}
			// 2.字符串进行填充到DocInfo
			DocInfo doc;
			doc.title = results[0];
			doc.content = results[1];
			doc.url = results[2];
			doc.doc_id = forward_index.size();
			// 3.插入到正排索引的vector
			forward_index.push_back(doc);
			return &forward_index.back();
		}
		bool BuildInvertedIndex(const DocInfo &doc)
		{
			// DocInfotitle, content, url, doc_id;
			// word -> 倒排拉链
			struct word_cnt
			{
				int title_cnt;
				int content_cnt;
				word_cnt()
					: title_cnt(0), content_cnt(0) {}
			};

			std::unordered_map<std::string, word_cnt> word_map; // 用来暂存词频映射表
			std::vector<std::string> title_words;

			// 对标题进行词频统计
			util::JiebaUtil::CutString(doc.title, &title_words);
			for (auto s : title_words)
			{
				boost::to_lower(s); // 将我们的分词统一转换为小写
				word_map[s].title_cnt++;
			}
			// 对内容进行词频统计
			std::vector<std::string> content_words;
			util::JiebaUtil::CutString(doc.content, &content_words);
			for (auto s : content_words)
			{
				boost::to_lower(s);
				word_map[s].content_cnt++;
			}

#define X 10
#define Y 1

			for (auto &word_pair : word_map)
			{
				InvertedElem it;
				it.doc_id = doc.doc_id;
				it.word = word_pair.first;
				it.weight = X * word_pair.second.title_cnt + Y * word_pair.second.content_cnt; // 相关性
				InvertedList &inverted_list = inverted_index[word_pair.first];
				inverted_list.push_back(it);
			}

			return true;
		}
	};

}
