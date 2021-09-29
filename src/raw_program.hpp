//raw_program.hpp: 管理控制器程序集
//使用安川提供的SDK直接导出控制器程序后，会得到一个csv文件，其中包含多个（如50个）
//控制器程序，我们称位于一个csv文件中的程序是一个“控制器程序集”。
//包含类raw_program_t
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
#pragma once
#include <map>
#include <vector>
#include <cstdint>
#include <filesystem>

class instruction_set_t;

//类名中使用raw是因为我获得的控制器程序集的样例的文件名是“原始文件.csv”，尽管这里的
//“原始”的更可能的含义是“original”，将其译为“raw”体现出csv文件中的程序是未经人类可
//读化处理的、晦涩的。
class raw_program_t
{
public:
	//必须在创建其他线程前调用
	static void set_program_length(const instruction_set_t&)noexcept;

	void load_null(unsigned first_index,unsigned size);
	//从csv文件中加载程序集，first_index是csv文件中位于首行的程序的实际索引
	void load_csv(const std::filesystem::path &path,unsigned first_index=0);
	//以打开时的first_index和csv文件的程序数保存程序
	void save_csv_origin(const std::filesystem::path &path);
	//将索引属于[index_begin,index_end)的程序保存到csv文件，csv文件中的首行的索引为index_begin，末行的索引为index_end-1
	//因某个索引对应的程序不存在而引起的空位用全0的程序填充
	//特别地，index_begin=-1代表最小的索引，index_end=-1代表最大的索引+1
	void save_csv(const std::filesystem::path &path,unsigned index_begin=unsigned(-1),unsigned index_end=unsigned(-1))const;
	//按顺序将索引介于[index_begin,index_end)的程序保存到csv文件，跳过空位，所以无法从csv文件的程序的行号还原出程序的索引
	//当索引属于[index_begin,index_end)的程序的数量不足min_size时，在csv文件的末尾填充全0的程序，以使csv文件的行数等于min_size
	void save_csv_skip(const std::filesystem::path &path,unsigned index_begin=0,unsigned index_end=unsigned(-1),unsigned min_size=0)const;
	const std::vector<int16_t>& get_raw_program(unsigned index)const
	{
		return rp.at(index);
	}
	
	void set_raw_program(unsigned index,const std::vector<int16_t> &p);
	
	bool reset_index(unsigned old_index,unsigned new_index)noexcept;
	void remove(unsigned index)
	{
		rp.erase(index);
	}
	
	bool has(unsigned index)const noexcept
	{
		return rp.count(index)!=0;
	}
	
	std::pair<unsigned,unsigned> index_range()const
	{
		return {first_index_origin,first_index_origin+size_origin};
	}
	
	auto cbegin()const noexcept
	{
		return rp.cbegin();
	}
	
	auto cend()const noexcept
	{
		return rp.cend();
	}
private:
	static size_t program_length;
	std::map<unsigned,std::vector<int16_t>> rp;
	unsigned first_index_origin,size_origin;
};
