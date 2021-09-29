//program_t.hpp: 管理单个控制器程序
//包含类program_t
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
#include <vector>
#include <filesystem>
#include <array>
#include <cstdint>
#include <memory>
#include "program_xlsx_style.hpp"

class instruction_set_t;
namespace xlnt{class worksheet;}


//一个程序
class program_t
{
public:
	program_t()=default;
	
	//必须在创建其他线程前调用
	static void set_instruction_set(const instruction_set_t &);
	
	//原始程序是导出的csv文件中的一行, 是有符号的16位整数
	void load_raw_program(const std::vector<int16_t>&);
	void save_raw_program(std::vector<int16_t>&)const;
	
	void save_xlsx(const std::filesystem::path &path,const program_xlsx_style &style=PXLSX_DEFAULT_STYLE,std::ostream *error_stream=nullptr)const;//保存为xlsx
	void load_xlsx(const std::filesystem::path &path,std::ostream *error_stream=nullptr);//从xlsx文件中加载
	void save_serialization(const std::filesystem::path &path)const;//序列化并保存
	void load_serialization(const std::filesystem::path &path);//从序列化文件中加载
	void save_source_code(const std::filesystem::path &path)const;//保存为源代码
	void load_source_code(const std::filesystem::path &path);//从源代码加载
private:
	static const uint32_t version=1;
	static std::unique_ptr<instruction_set_t> instr_set;
//	static std::shared_mutex instr_set_mutex;
	template<bool>
	void save_xlsx_one_instr(xlnt::worksheet&,const program_xlsx_style &style,size_t,std::ostream*,void*)const;
	void load_xlsx_one_instr(xlnt::worksheet&,bool is_comment_left,size_t,std::vector<uint16_t>&,std::ostream*)const;
	std::vector<std::vector<uint16_t>> bin;
};
