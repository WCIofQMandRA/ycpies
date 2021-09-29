//throw_assert.hpp: 提供在Release模式下仍生效的断言
//这意味着，这种断言的目的不是调试，而是检查输入是否合法，断言失败将抛出异常
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
#include <string>
#include <type_traits>
#include <exception>

#define throw_assert(condition,type,message)\
	(static_cast<bool>(condition)?(void)0:\
	throw_assert_impl<type>(message,#condition,__FILE__,__LINE__,__func__))

template<typename ExceptionType>
[[noreturn]] inline void throw_assert_impl(const std::string &message,const std::string &source,const std::string &file,int line,const std::string &function)
{
	static_assert(std::is_convertible<ExceptionType*,std::exception*>::value,"ExceptionType must be derived from std::exception.");
	throw ExceptionType(file+":"+std::to_string(line)+": "+function+":  Assertion `"+source+"` failed!\n"+message);
}
