//my_exceptions.hpp: 非GUI类instruction_set_t、program_t和raw_program_t
//的成员函数可能抛出的异常
//包含类file_format_error、类cannot_open_file_error
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
#include <stdexcept>

//文件格式错误
class file_format_error:public std::runtime_error
{
public:
	file_format_error(const std::string& what_arg):
		std::runtime_error(what_arg){}
	file_format_error(const char *what_arg):
		std::runtime_error(what_arg){}
};

//无法打开文件
class cannot_open_file_error:public std::runtime_error
{
public:
	cannot_open_file_error(const std::string& what_arg):
		std::runtime_error(what_arg){}
	cannot_open_file_error(const char *what_arg):
		std::runtime_error(what_arg){}
};
