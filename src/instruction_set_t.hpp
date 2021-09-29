//instruction_set_t.hpp: 管理控制器程序使用的指令集
//包含结构体argument_t、结构体instruction_t、类instruction_set_t
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
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

//定点数的放大倍律，设x是实数，则存储最接近 x*FIXED_POINT_RATE 的整数
const double FIXED_POINT_RATE=1000;//??? FIXME

//指令中的一个参数的含义
struct argument_t
{
	std::string description;//参数描述，如 各轴参数引用方式
	//参数的长度，有效的参数的length至少为1；
	//length=0表示前一个参数的length>=2，当前位置是前一个参数的一部分
	//length=-1表示该位置不应该有参数
	int length;
	enum:unsigned//参数的类型
	{
		MARK_SET=0xFF00,
		TPS_NUM=0x0100,
		TPS_UNUM=0x0200,
		TP_INT=0x0101,     //有符号整数
		TP_FIXED=0x0102,   //定点数
		TP_UINT=0x0201,    //无符号整数
		TP_BIN=0x0202,     //用二进制输出的整数
		TP_HEX=0x0203,     //用十六进制输出的整数
		TP_STR=0x0301,     //字符串, 编码用UTF-8
		TP_STRW=0x0401     //字符串，编码用UTF-16
	}type;
};

//单种指令
struct instruction_t
{
//	unsigned ID;//指令代号，如 4
	std::string abbr;//指令缩写，如 MVS
	std::string name;//指令名称，如 多轴连动
	std::vector<argument_t> args;//指令参数，args[0]留空，有效下标从1开始，因为MW0已被用作指令代号
};

//指令集
class instruction_set_t
{
public:
	//从csv文件加载指令集, 强异常保证
	//当无法打开文件时返回cannot_open_file_error型异常
	//当文件格式错误时抛出file_format_error型异常
	void load_csv(const std::filesystem::path &path);
	
	void save_csv(const std::filesystem::path &path)const;//将指令集保存到csv文件
	void save_serialization(const std::filesystem::path &path)const;//序列化并保存
	void load_serialization(const std::filesystem::path &path);//从序列化文件中加载
	
	const instruction_t& at(unsigned instruction_ID)const
	{
		return instructions.at(instruction_ID);
	}
	
	bool has(unsigned instruction_ID)const noexcept
	{
		return instructions.count(instruction_ID)!=0;
	}
	
	//获取单条指令的长度
	size_t one_instruction_length()const noexcept
	{
		return instructions.size()?instructions.begin()->second.args.size():0;
	}
	
	//获取一个程序中指令的总数
	size_t instruction_count()const noexcept
	{
		return instruction_cnt;
	}
	
	//通过指令缩写获得指令代号
	unsigned instruction_ID(const std::string &abbr)const
	{
		return abbr_to_ID.at(abbr);
	}
	
private:
	void generate_abbr_to_ID()noexcept;
	static const uint32_t version=1;
	std::map<unsigned,instruction_t> instructions;
	std::map<std::string,unsigned> abbr_to_ID;
	size_t instruction_cnt=0;
};
