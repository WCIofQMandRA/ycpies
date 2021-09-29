//program_xlsx_style.hpp 
//包含结构体program_xlsx_style
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
#include <cstdint>
constexpr uint8_t PXLSX_COMMENT_LEFT=1;
constexpr uint8_t PXLSX_COMMENT_RIGHT=0;
constexpr uint8_t PXLSX_COMMENT_COLON=2;
constexpr uint8_t PXLSX_COMMENT_LCOLON=3;//3=2|1

constexpr double PXLSX_WIDTH_V0_1=7.5;//version 0.1的宽度
constexpr double PXLSX_WIDTH_AUTO=-0.1;//任何小于0的值都表示自动计算宽度，但其相反数表示最小的宽度

#ifdef __linux__
constexpr double PXLSX_WIDTH_MULTI_DEFAULT=1.85;
#elif defined(_WIN32)
constexpr double PXLSX_WIDTH_MULTI_DEFAULT=1.5;
#else
constexpr double PXLSX_WIDTH_MULTI_DEFAULT=1;
#endif

//导出的xlsx文件的风格
struct program_xlsx_style
{
	uint8_t comment;//参数的注释信息的位置（左侧/右侧/左侧带冒号）
	double width;//单元格宽度（单位不详）
	double width_auto_multi;//自动计算单元格宽度时，宽度的乘数，或者说是em到xlnt中单元格的宽度单位的转换倍率
};

constexpr program_xlsx_style PXLSX_DEFAULT_STYLE{PXLSX_COMMENT_LCOLON,PXLSX_WIDTH_AUTO,PXLSX_WIDTH_MULTI_DEFAULT};
