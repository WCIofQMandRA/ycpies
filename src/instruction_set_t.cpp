//instruction_set_t.cpp
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
#include "instruction_set_t.hpp"
#include "throw_assert.hpp"
#include "my_exceptions.hpp"
#include <fstream>
#include <utility>
#include <libcsv/csv.h>

static void cb1(void *s,size_t len,void *out)
{
	auto a=reinterpret_cast<std::pair<std::vector<std::vector<std::string>>,std::vector<std::string>>*>(out);
	a->second.push_back(std::string(reinterpret_cast<char*>(s),len));
}

static void cb2 (int,void *out)
{
	auto a=reinterpret_cast<std::pair<std::vector<std::vector<std::string>>,std::vector<std::string>>*>(out);
	a->first.push_back(std::move(a->second));
	a->second.clear();
}

static void load_csv_to_vector(std::vector<std::vector<std::string>> &out,const std::filesystem::path &path)
{
	std::ifstream fin(path,std::ios::binary);
	throw_assert(fin,cannot_open_file_error,path.u8string());
	char buf[1024];
	size_t bytes_read;
	csv_parser pa;
	csv_init(&pa,0);
	std::pair<std::vector<std::vector<std::string>>,std::vector<std::string>> tmp_pair;
	while((bytes_read=(fin.read(buf,1024),fin.gcount()))>0)
	{
		if(csv_parse(&pa,buf,bytes_read,cb1,cb2,&tmp_pair)!=bytes_read)
		{
			file_format_error err(csv_strerror(csv_error(&pa)));
			csv_free(&pa);
			throw err;
		}
	}
	csv_fini(&pa,cb1,cb2,NULL);
	fin.close();
	csv_free(&pa);
	out=std::move(tmp_pair.first);
}

//从csv文件加载指令集
void instruction_set_t::load_csv(const std::filesystem::path &path)
{
	std::map<unsigned,instruction_t> instructions_new;
	size_t instruction_cnt_new=0;
	std::vector<std::vector<std::string>> csv_file;
	load_csv_to_vector(csv_file,path);
	
	unsigned length_one_instruction=0;//单个指令的长度，等于 (首行的有效长度-2)
	unsigned row=1,col=1;
#define SRC_INDEX "R"+std::to_string(row)+"C"+std::to_string(col)+": "
	
	throw_assert(csv_file.size()>=2,file_format_error,"指令集至少应该有2行.");
	throw_assert(csv_file[0].size()>=3,file_format_error,"指令集的首行至少应该有3列.");
	for(size_t j=csv_file[0].size()-1;j>=2;--j)
	{
		if(csv_file[0][j]!="")
		{
			length_one_instruction=j-1;
			break;
		}
	}
	for(size_t j=2;j<length_one_instruction+2;++j)
	{
		throw_assert(csv_file[0][j]=="MW"+std::to_string(j-2),file_format_error,SRC_INDEX "首行的格式不正确.");
	}
	
	row=2;
	for(auto i=csv_file.begin()+1;i!=csv_file.end();++i,++row)
	{
		if(!i->size())continue;
		//指令代号为TOTAL，则下一个单元格为一个程序中的指令的总数
		//以TOTAL开头的行被视为最后一行
		if((*i)[0]=="TOTAL")
		{
			try{instruction_cnt_new=std::stoul(i->at(1));}
			catch(std::exception &e)
			{
				col=2;
				throw file_format_error(SRC_INDEX+e.what());
			}
			break;
		}
		else
		{
			col=1;
			throw_assert(i->size()>=3,file_format_error,SRC_INDEX "一行至少要含有三列.");
			//获得指令代号
			instruction_t instr;
			unsigned ID=0;//指令代号
			try{ID=std::stoul((*i)[0]);}
			catch(std::exception &e)
			{
				throw file_format_error(SRC_INDEX+e.what());
			}
			throw_assert(!instructions_new.count(ID),file_format_error,SRC_INDEX "重复的指令代号.");
			
			instr.abbr=(*i)[1];//指令缩写
			instr.name=(*i)[2];//指令名称
			
			//读取MW1~(length_one_instruction-1)的描述
			instr.args.resize(length_one_instruction);
			for(auto &j:instr.args)
				j.length=-1;
			for(col=4;col<=i->size();++col)
			{
				if(!(*i)[col-1].size())continue;//可能一：该位置不应该有参数; 可能二: 上一个位置的参数的长度>=2; 可能三: 行尾多余的空单元格
				throw_assert(col-3<length_one_instruction,file_format_error,SRC_INDEX "参数的数量过多.");
				if(instr.args[col-3].length==0)continue;//上一个位置的参数的长度>=2
				
				std::string s=(*i)[col-1];
				if(s[s.size()-1]=='$')//s中含有附加信息
				{
					size_t p=s.size()-2;
					while(p&&s[p]!='$')
						--p;
					throw_assert(s[p]=='$',file_format_error,SRC_INDEX "找不到附加信息描述的开头.");
					auto append=s.substr(p+1);
					append.resize(append.size()-1);
					s.resize(p);
					
					//分析附加参数
					//附加参数的格式：
					//若干由逗号分割的字段
					//如果字段以大写字母开头，则该字段描述参数的类型，接受的值：INT, UINT, STR, FIXED, 默认值:INT
					//如果字段以数字开头，则该字段描述参数的长度，接受的值: 1, 2, ... , (length_one_instruction-1)
					//否则，该字段必须是<key> = <value>的形式，`key`完全由小写字母组成，<value>使用C字符串的转义方式书写，除了逗号需要写成\,
					//目前，尚无任何可用的<key>
					instr.args[col-3].length=1,instr.args[col-3].type=argument_t::TP_INT;
					auto j=append.begin();
					while(j!=append.end())
					{
						if(*j>='A'&&*j<='Z')
						{
							std::string word;
							while(j!=append.end()&&*j!=',')
								word+=*j++;
							if(j!=append.end())++j;
							if(word=="INT")instr.args[col-3].type=argument_t::TP_INT;
							else if(word=="UINT")instr.args[col-3].type=argument_t::TP_UINT;
							else if(word=="FIXED")instr.args[col-3].type=argument_t::TP_FIXED;
							else if(word=="BIN")instr.args[col-3].type=argument_t::TP_BIN;
							else if(word=="HEX")instr.args[col-3].type=argument_t::TP_HEX;
							else if(word=="STR")instr.args[col-3].type=argument_t::TP_STR;
							else if(word=="STRW")instr.args[col-3].type=argument_t::TP_STRW;
							else throw file_format_error(SRC_INDEX "未知的参数类型 "+word);
						}
						else if(*j>='0'&&*j<='9')
						{
							int len=0;
							while(j!=append.end()&&*j!=',')
							{
								char ch=*j++;
								throw_assert(ch>='0'&&ch<='9',file_format_error,SRC_INDEX "参数长度必须只含数字.");
								len=len*10+ch-48;
							}
							if(j!=append.end())++j;
							instr.args[col-3].length=len;
							for(unsigned k=col-2;k<col-3+len;++k)
								instr.args.at(k).length=0;
						}
						else throw file_format_error(SRC_INDEX "未知的附加参数.");
					}
				}
				//不含附加信息，取默认值
				else instr.args[col-3].length=1,instr.args[col-3].type=argument_t::TP_INT;
				instr.args[col-3].description=s;
			}
			
			//将instr存入指令集中
			instructions_new[ID]=std::move(instr);
		}
	}
	throw_assert(instruction_cnt_new,file_format_error,"文件中不包含一个程序中指令的总数信息.");
	instructions=std::move(instructions_new);
	instruction_cnt=instruction_cnt_new;
	generate_abbr_to_ID();
#undef SRC_INDEX
}

void instruction_set_t::generate_abbr_to_ID()noexcept
{
	abbr_to_ID.clear();
	for(auto &i:instructions)
		abbr_to_ID[i.second.abbr]=i.first;
}
