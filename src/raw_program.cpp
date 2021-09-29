//raw_program.cpp
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
#include "raw_program.hpp"
#include <fstream>
#include <cstdio>
#include "throw_assert.hpp"
#include "my_exceptions.hpp"
#include "instruction_set_t.hpp"


size_t raw_program_t::program_length;

void raw_program_t::set_program_length(const instruction_set_t &s)noexcept
{
	program_length=s.one_instruction_length()*s.instruction_count();
}

bool raw_program_t::reset_index(unsigned old_index,unsigned new_index)noexcept
{
	if(!rp.count(old_index))return false;
	if(rp.count(new_index))
	{
		std::swap(rp[old_index],rp[new_index]);
	}
	else
	{
		std::swap(rp[old_index],rp[new_index]);
		rp.erase(old_index);
	}
	return true;
}

void raw_program_t::set_raw_program(unsigned index,const std::vector<int16_t> &p)
{
	throw_assert(p.size()==program_length,std::runtime_error,"程序长度错误.");
	rp[index]=p;
}

void raw_program_t::load_null(unsigned first_index,unsigned size)
{
	rp.clear();
	first_index_origin=first_index;
	size_origin=size;
}

void raw_program_t::load_csv(const std::filesystem::path &path,unsigned first_index)
{
	std::ifstream pset_file(path,std::ios::binary);
	throw_assert(pset_file,cannot_open_file_error,path.u8string());
	std::string str_line;
	unsigned n_program=first_index;
	std::map<unsigned,std::vector<int16_t>> rp_new;
	while(std::getline(pset_file,str_line))
	{
		if(!str_line.size())
			continue;
		if(str_line[str_line.size()-1]=='\r')
			str_line.resize(str_line.size()-1);
		if(!str_line.size())
			continue;
		int x=0;
		bool neg=false,done=true;
		std::vector<int16_t> arr_line;
		for(auto i:str_line)
		{
			if(i==',')
			{
				arr_line.push_back(static_cast<int16_t>(neg?-x:x));
				x=0,neg=false,done=true;
			}
			else if(i=='-')
				neg=true,done=false;
			else if(i>='0'&&i<='9')
			{
				done=false;
				x=x*10+(i-48);
			}
			else
				throw file_format_error(std::string("意外的字符: ")+i+"("+std::to_string((unsigned)(uint8_t)(i))+")");
		}
		if(!done)
			arr_line.push_back(static_cast<int16_t>(neg?-x:x));
		throw_assert(arr_line.size()==program_length,file_format_error,"程序长度错误.");
		if([&arr_line]
		{
			for(auto i:arr_line)
				if(i)return true;
			return false;
		}())
		{
			rp_new[n_program]=std::move(arr_line);
		}
		++n_program;
	}
	rp=std::move(rp_new);
	first_index_origin=first_index;
	size_origin=n_program-first_index;
}

void raw_program_t::save_csv_origin(const std::filesystem::path &path)
{
	save_csv(path,first_index_origin,first_index_origin+size_origin);
}

void raw_program_t::save_csv(const std::filesystem::path &path,unsigned index_begin,unsigned index_end)const
{
	if((index_begin==-1||index_end==-1)&&!rp.size())return;
	if(index_begin==unsigned(-1))
		index_begin=rp.begin()->first;
	if(index_end==unsigned(-1))
		index_end=(--rp.end())->first+1;

#ifndef _WIN32
	FILE *fp=std::fopen(path.c_str(),"wb");
#else
	FILE *fp=_wfopen(path.c_str(),L"wb");
#endif
	throw_assert(fp,cannot_open_file_error,path.u8string());
	for(unsigned i=index_begin;i!=index_end;++i)
	{
		if(auto it=rp.find(i);it!=rp.end())
		{
			for(auto j:it->second)
				std::fprintf(fp,"%06d,",j);
		}
		else
		{
			for(size_t j=0;j<program_length;++j)
				std::fprintf(fp,"000000,");
		}
		//去除行末的逗号. RFC4180的示例中，行末没有逗号.
		fseek(fp,-1,SEEK_CUR);
		//RFC4180 : Each record is located on a separate line, delimited by a line break (CRLF).
		std::fprintf(fp,"\r\n");
	}
	std::fclose(fp);
}

void raw_program_t::save_csv_skip(const std::filesystem::path &path,unsigned index_begin,unsigned index_end,unsigned min_size)const
{
	if(!rp.size())return;
	auto begin=rp.lower_bound(index_begin),end=rp.upper_bound(index_end);
	unsigned x=0;
	
	#ifndef _WIN32
	FILE *fp=std::fopen(path.c_str(),"wb");
#else
	FILE *fp=_wfopen(path.c_str(),L"wb");
#endif
	throw_assert(fp,cannot_open_file_error,path.u8string());
	for(auto i=begin;i!=end;++i,++x)
	{
		for(auto j:i->second)
			std::fprintf(fp,"%06d,",j);
		fseek(fp,-1,SEEK_CUR);
		std::fprintf(fp,"\r\n");
	}
	for(;x<min_size;++x)
	{
		for(size_t j=0;j<program_length;++j)
			std::fprintf(fp,"000000,");
		fseek(fp,-1,SEEK_CUR);
		std::fprintf(fp,"\r\n");
	}
	std::fclose(fp);
}
