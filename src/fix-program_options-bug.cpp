//fix-program_options-bug.cpp
//Copyright (C) 2021 张子辰 <zichen350@gmail.com>

// 本文件是软件安川控制器程序导入导出系统 (Yaskawa Controller Program Import
// and Export System, ycpies, 以下简称“本软件”)的一部分。
// 在遵守以下条款的前提下，允许以任何方式和任何目的，运行、复制、分发和/或修改本
// 软件或将本软件与其他软件组合使用：
// 1. 在发布修改后的本软件或本软件的衍生作品的源代码时，在源代码中保留版权声明、
// 使用条款和免责声明。
// 2. 在发布修改后的本软件或本软件的衍生作品的二进制可执行文件时，以人类可读的方
// 式提及本软件的原作者和原作者发布本软件时使用的软件名称。
// 3. 不得以原作者的名义发布修改后的本软件或本软件的衍生作品；在发布修改后的本软
// 件时，应避免修改后的本软件被误认为是未经修改的本软件。
// 4. 除非技术上不可行或法律上不允许，使用(不含BOM的)UTF-8编码保存修改后的本软件
// 或本软件的衍生作品的源代码。

// 【免责声明】本软件“照原样”提供，不提供任何担保，包括但不限于适销性、适合于特定的
// 目的和不侵权。对于使用本软件所引起的任何损失、纠纷或侵权，本软件的作者概不负责。

#include <boost/program_options.hpp>
#include <iostream>
namespace po=boost::program_options;

//简陋的Unicode渲染支持，只能处理CJK和Latin字符。
//获取当前的码位，用UTF-32表示
//FIXME: 当第一字节是110yyyyy且第二字节是0xxxxxxx时，只将第一字节换成0xFFFD
//template<typename Tp,typename=enable_if_t<is_same_v<Tp,std::string::iterator>|is_same_v<Tp,std::string::const_iterator>>>
char32_t get_code_point(const std::string::const_iterator &it,int *units=nullptr)
{
	unsigned char ch=*it;
	if(ch<128u)
	{
		if(units!=nullptr)*units=1;
		return ch;
	}
	//10xxxxxx
	if(ch<192u)
	{
		if(units!=nullptr)*units=1;
		return 0xFFFD;
	}
	//110yyyyy 10xxxxxx
	if(ch<224u)
	{
		unsigned char ch1=*(it+1);
		if(ch1<128u||ch1>=192u)
		{
			if(units!=nullptr)*units=2;
			return 0xFFFD;
		}
		if(units!=nullptr)*units=2;
		return char32_t(ch&31u)<<6|char32_t(ch1&63u);
	}
	//1110zzzz 10yyyyyy 10xxxxxx
	if(ch<240u)
	{
		unsigned char ch1=*(it+1);
		if(ch1<128u||ch1>=192u)
		{
			if(units!=nullptr)*units=2;
			return 0xFFFD;
		}
		unsigned char ch2=*(it+2);
		if(ch2<128u||ch2>=192u)
		{
			if(units!=nullptr)*units=3;
			return 0xFFFD;
		}
		if(units!=nullptr)*units=3;
		return char32_t(ch&15u)<<12|char32_t(ch1&63u)<<6|char32_t(ch2&63u);
	}
	//11110uuu 10zzzzzz 10yyyyyy 10xxxxxx
	if(ch<248u)
	{
		unsigned char ch1=*(it+1);
		if(ch1<128u||ch1>=192u)
		{
			if(units!=nullptr)*units=2;
			return 0xFFFD;
		}
		unsigned char ch2=*(it+2);
		if(ch2<128u||ch2>=192u)
		{
			if(units!=nullptr)*units=3;
			return 0xFFFD;
		}
		unsigned char ch3=*(it+3);
		if(ch3<128u||ch3>=192u)
		{
			if(units!=nullptr)*units=4;
			return 0xFFFD;
		}
		if(units!=nullptr)*units=4;
		return char32_t(ch&7u)<<18|char32_t(ch1&63u)<<12|char32_t(ch2&63u)<<6|char32_t(ch3&63u);
	}
	if(units!=nullptr)*units=1;
	return 0xFFFD;
}

//将迭代器增加到下一个码位
//要求：1. string的是有效的UTF-8字符串；2. it应指向一个码位的起始位置；3. it可以安全地解引用
//不满足要求时行为未定义
//返回值：若增加成功，则返回增加的距离；若失败，则返回增加的距离的相反数
//template<typename Tp,typename=enable_if_t<is_same_v<Tp,std::string::iterator>|is_same_v<Tp,std::string::const_iterator>>>
ptrdiff_t next_code_point(std::string::const_iterator &it)
{
	unsigned char ch=*it++;
	//0xxxxxxx
	if(ch<128u)return 1;
	//10xxxxxx
	if(ch<192u)return -1;
	//110xxxxx
	if(ch<224u)
	{
		ch=*it++;
		if(ch<128u||ch>=192u)return -2;
		return 2;
	}
	//1110xxxx
	if(ch<240u)
	{
		ch=*it++;
		if(ch<128u||ch>=192u)return -2;
		ch=*it++;
		if(ch<128u||ch>=192u)return -3;
		return 3;
	}
	//11110xxx
	if(ch<248u)
	{
		ch=*it++;
		if(ch<128u||ch>=192u)return -2;
		ch=*it++;
		if(ch<128u||ch>=192u)return -3;
		ch=*it++;
		if(ch<128u||ch>=192u)return -4;
		return 4;
	}
	return -1;
}

size_t get_width(char32_t code_point)
{
	if(0x4E00<=code_point&&code_point<=0x9FFF)return 2;
	if(0x3400<=code_point&&code_point<=0x4DBF)return 2;//A
	if(0x20000<=code_point&&code_point<=0x2A6DF)return 2;//B
	if(0x2A700<=code_point&&code_point<=0x2B73F)return 2;//C
	if(0x2B740<=code_point&&code_point<=0x2B81F)return 2;//D
	if(0x2B820<=code_point&&code_point<=0x2CEAF)return 2;//E
	if(0x2CEB0<=code_point&&code_point<=0x2EBEF)return 2;//F
	if(0x30000<=code_point&&code_point<=0x3134F)return 2;//G
	if(0xF900<=code_point&&code_point<=0xFAFF)return 2;
	if(0x2F800<=code_point&&code_point<=0x2FA1F)return 2;
	return 1;
}

//the line width of discription should be long enough to avoid automatically line-breaking
void print_options_decription(std::ostream &out,const po::options_description &discription,
	unsigned line_length=po::options_description::m_default_line_length,
	unsigned min_decription_length=po::options_description::m_default_line_length/2)
{
	using namespace std;
	ostringstream sout;
	discription.print(sout);
	string str=sout.str();
	auto width=discription.get_option_column_width();
	vector<pair<string,string>> options;
	vector<size_t> per_option_width;
	size_t option_column_width=0;
	size_t i=0;
	while(i<str.length())
	{
		size_t j=i;
		while(j<str.length()&&str[j]!='\n')++j;
		if(j-i<=width)
		{
			options.emplace_back(str.substr(i,j-i),"");//不带说明的选项
			i=j+1;
			continue;
		}
		size_t option_width=i+width;
		while(str[--option_width]==' ');
		options.emplace_back(str.substr(i,option_width-i+1),str.substr(i+width,j-(i+width)));
		i=j+1;
	}
	for(const auto &i:options)
	{
		size_t cnt=0;
		for(auto j=i.first.begin();j!=i.first.end();next_code_point(j))
			cnt+=get_width(get_code_point(j));
		per_option_width.emplace_back(cnt);
		option_column_width=max(option_column_width,cnt);
	}
	if(option_column_width<min_decription_length)option_column_width=min_decription_length;
	if(option_column_width>line_length/2)option_column_width=line_length/2;
	for(size_t i=0;i<options.size();++i)
	{
		out<<options[i].first;
		if(per_option_width[i]>=option_column_width)
		{
			out<<"\n";
			for(size_t j=0;j<option_column_width;++j)out<<' ';
		}
		else
		{
			for(size_t j=per_option_width[i];j<option_column_width;++j)out<<' ';
		}
		size_t cnt=0,left_width=line_length-option_column_width;
		for(auto j=options[i].second.begin();j!=options[i].second.end();)
		{
			int units;
			cnt+=get_width(get_code_point(j,&units));
			if(cnt<=left_width)
			{
				for(int k=0;k<units;++k)out<<*j++;
			}
			else
			{
				out<<'\n';
				for(size_t k=0;k<option_column_width;++k)out<<' ';
				for(int k=0;k<units;++k)out<<*j++;
				cnt=0;
			}
		}
		out<<endl;
	}
}
