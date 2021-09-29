//program_t.cpp
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
#include "program_t.hpp"
#include <cstring>
#include "my_exceptions.hpp"
#include <xlnt/xlnt.hpp>
#include "throw_assert.hpp"
#include <algorithm>
#include <utf8/cpp11.h>
#include <fstream>
#include <stdexcept>
#include "text_width.hpp"
#include <map>
//#include <iostream>

//由于xlnt提供的查询合并单元格的API不合适，自行维护被合并的单元格
//first: 被合并的单元格的所在的大单元格包含的单元格的make_pair(列数,行数)
//second: 被合并的单元格所在的大单元格的左上角的单元格的索引
//这些信息用于确定在计算单元格宽度时的权重
//单元格的宽度权重=1.0/合并的列的数量
//为了减小储存空间，合并数为1的单元格不储存
//TODO: 用专门的类维护
//typedef std::pair
//	<
//		std::map<std::pair<size_t,size_t>,std::pair<int,int>>,
//		std::map<std::pair<size_t,size_t>,std::pair<size_t,size_t>>
//	> merged_cells_t;

typedef std::vector<double> colums_width_t;

std::unique_ptr<instruction_set_t> program_t::instr_set;
//std::shared_mutex program_t::instr_set_mutex;

static std::string to_bin(unsigned long long x)
{
	if(!x)return "0b0";
	std::string y;
	while(x)
	{
		y+=(x%2+'0');
		x/=2;
	}
	y+="b0";
	std::reverse(y.begin(),y.end());
	return y;
}

static std::string to_hex(unsigned long long x)
{
	if(!x)return "0x0";
	std::string y;
	while(x)
	{
		unsigned z=x%16;
		x/=16;
		y+=(z<10?z+'0':z+'A'-10);
	}
	y+="x0";
	std::reverse(y.begin(),y.end());
	return y;
}

static unsigned long long my_stoull(const std::string &s)
{
	if(!s.size())return 0;
	unsigned long long y=0;
	bool neg=false;
	auto i=s.begin();
	if(*i=='-')neg=true,++i;
	if(*i=='0')
	{
		++i;
		if(i==s.end())return 0;
		if(*i=='x'||*i=='X')
		{
			++i;
			while(i!=s.end())
			{
				char ch=*i++;
				if(ch>='0'&&ch<='9')
					y=y*16+(ch-'0');
				else if(ch>='A'&&ch<='F')
					y=y*16+(ch-'A'+10);
				else if(ch>='a'&&ch<='f')
					y=y*16+(ch-'a'+10);
				else
					throw 0;
			}
		}
		else if(*i=='b'||*i=='B')
		{
			++i;
			while(i!=s.end())
			{
				char ch=*i++;
				if(ch=='0')
					y<<=1;
				else if(ch=='1')
					y=y<<1|1;
				else
					throw 0;
			}
		}
		else
		{
			while(i!=s.end())
			{
				char ch=*i++;
				if(ch>='0'&&ch<='7')
					y=y*8+(ch-'0');
				else
					throw 0;
			}
		}
	}
	else
	{
		while(i!=s.end())
		{
			char ch=*i++;
			if(ch>='0'&&ch<='9')
				y=y*10+(ch-'0');
			else
				throw 0;
		}
	}
	if(neg)
#ifdef _MSC_VER
		y=~(y-1);
#else
		y=-y;
#endif
	return y;
}

void program_t::set_instruction_set(const instruction_set_t &instruction_set)
{
//	std::unique_lock _(instr_set_mutex);
	if(!instr_set)
		instr_set=std::make_unique<instruction_set_t>(instruction_set);
	else
		*instr_set=instruction_set;
}

void program_t::load_raw_program(const std::vector<int16_t> &raw)
{
//	std::shared_lock _(instr_set_mutex);
	throw_assert(instr_set,std::runtime_error,"尚未加载指令集.");
	const auto cnt=instr_set->instruction_count(),len=instr_set->one_instruction_length();
//	_.unlock();
	throw_assert(raw.size()==cnt*len,std::runtime_error,"程序大小错误.");
	for(size_t i=0;i<cnt;++i)
	{
		std::vector<uint16_t> ins(len);
		std::memcpy(ins.data(),raw.data()+i*len,2*len);//sizeof(int16_t)=2
		//ins不是全0的
		if([&ins]{
			for(auto j:ins)
			{
				if(j!=0)return true;
			}
			return false;}())
		{
			if(bin.size()!=i)bin.resize(i,std::vector<uint16_t>(len));
			bin.push_back(std::move(ins));
		}
	}
}

void program_t::save_raw_program(std::vector<int16_t> &raw)const
{
//	std::shared_lock _(instr_set_mutex);
	throw_assert(instr_set,std::runtime_error,"尚未加载指令集.");
	const auto cnt=instr_set->instruction_count(),len=instr_set->one_instruction_length();
//	_.unlock();
	raw.resize(cnt*len,0);
	for(size_t i=0;i<bin.size();++i)
		std::memcpy(raw.data()+i*len,bin[i].data(),2*len);
}

template<typename T>
double set_value(xlnt::cell &&cell,const T &x)
{
	cell.value(x);
	return get_text_display_size(std::to_string(x));
}

template<>
double set_value<std::string>(xlnt::cell &&cell,const std::string &x)
{
	cell.value(x);
	return get_text_display_size(x);
}

template<bool auto_width>
void program_t::save_xlsx_one_instr(xlnt::worksheet &ws,const program_xlsx_style &style,size_t ssss,std::ostream *error_stream,void *extra)const
{
	xlnt::row_t row(ssss+2);
	auto &ins=bin[ssss];
	auto &ins_info=instr_set->at(ins[0]);
	const auto width=instr_set->one_instruction_length();
	auto &colums_width=*reinterpret_cast<colums_width_t*>(extra);

//	if(ins_info==instructions.end())
//	{
//		ws.merge_cells(xlnt::range_reference(1,row,width,row));
//		ws.merge_cells(xlnt::range_reference(width+1,row,width*2,row));
//		ws.cell(1,row).value("[UNKNOWN]");
//		ws.cell(1,row).fill(xlnt::fill::solid(xlnt::color::red()));
//		ws.cell(width+1,row).fill(xlnt::fill::solid(xlnt::rgb_color(192u,192u,192u)));
//		return;
//	}
	{
		int c1=2,c2=1;
		if(!(style.comment&PXLSX_COMMENT_LEFT))
			std::swap(c1,c2);
		if constexpr(auto_width)
			colums_width[c1]=std::max(colums_width[c1],set_value(ws.cell(c1,row),ins_info.abbr));
		else
			ws.cell(c1,row).value(ins_info.abbr);
		if constexpr(auto_width)
			colums_width[c2]=std::max(colums_width[c2],set_value(ws.cell(c2,row),ins_info.name+((style.comment&PXLSX_COMMENT_COLON)?":":"")));
		else
			ws.cell(c2,row).value(ins_info.name+((style.comment&PXLSX_COMMENT_COLON)?":":""));
		ws.cell(c2,row).fill(xlnt::fill::solid(xlnt::rgb_color(192u,192u,192u)));
	}
	
	for(size_t i=1;i<ins.size();++i)
	{
		int len=ins_info.args[i].length;
		if(len==0)continue;
		if(len==-1)
		{
			xlnt::cell_reference value_cell_ref(i*2+2,row),comment_cell_ref(i*2+1,row);
			if(!(style.comment&PXLSX_COMMENT_LEFT))
				std::swap(value_cell_ref,comment_cell_ref);
			if(ins[i]!=0)
			{
#define VALUE(x,y,z)\
do\
{\
	if constexpr(auto_width)\
	{\
		if(z==1)\
			colums_width[x.column_index()]=std::max(colums_width[x.column_index()],set_value(ws.cell(x),y));\
		else\
		{\
			auto n=x.column_index()+z;\
			auto width=set_value(ws.cell(x),y)/z;\
			for(auto i=x.column_index();i<n;++i)\
				colums_width[i]=std::max(colums_width[i],width);\
		}\
	}\
	else\
		ws.cell(x).value(y);\
}while(false)
				VALUE(value_cell_ref,ins[i],1);
				ws.cell(value_cell_ref).fill(xlnt::fill::solid(xlnt::color::red()));
				if(error_stream)
					*error_stream<<"警告:\t指令 #"<<ssss+1<<": 指令 "<<ins_info.abbr<<" 的参数 MW"<<i<<" 无意义.\n"<<std::endl;
			}
			else
				ws.cell(value_cell_ref).fill(xlnt::fill::solid(xlnt::color::black()));
			ws.cell(comment_cell_ref).fill(xlnt::fill::solid(xlnt::color::black()));
			continue;
		}
		if(len>=2)
		{
			ws.merge_cells(xlnt::range_reference(i*2+1,row,i*2+len,row));
			ws.merge_cells(xlnt::range_reference(i*2+len+1,row,i*2+len*2,row));
		}
		
		xlnt::cell_reference value_cell_ref(i*2+len+1,row),comment_cell_ref(i*2+1,row);
			if(!(style.comment&PXLSX_COMMENT_LEFT))
				std::swap(value_cell_ref,comment_cell_ref);
		if((ins_info.args[i].type&argument_t::MARK_SET)==argument_t::TPS_UNUM)
		{
			unsigned long long rawdata;
			switch(len)
			{
			case 1:rawdata=ins[i];break;
			case 2:rawdata=static_cast<uint32_t>(ins[i+1])<<16|ins[i];break;
			case 4:rawdata=static_cast<uint64_t>(ins[i+3])<<48|static_cast<uint64_t>(ins[i+2])<<32
				|static_cast<uint64_t>(ins[i+1])<<16|ins[i];break;
			//default:ws.cell(i*2+1,row).value("整数的长度必须是1/2/4.");//TODO 在输入指令集时检测
			}
			xlnt::alignment algi;
			algi.horizontal(xlnt::horizontal_alignment::right);
			switch(ins_info.args[i].type)
			{
			case argument_t::TP_UINT:VALUE(value_cell_ref,rawdata,len);break;
			case argument_t::TP_BIN:VALUE(value_cell_ref,to_bin(rawdata),len);ws.cell(value_cell_ref).alignment(algi);break;
			case argument_t::TP_HEX:VALUE(value_cell_ref,to_hex(rawdata),len);ws.cell(value_cell_ref).alignment(algi);break;
			}
		}
		else if((ins_info.args[i].type&argument_t::MARK_SET)==argument_t::TPS_NUM)
		{
			long long rawdata;
			switch(len)
			{
			case 1:rawdata=static_cast<int16_t>(ins[i]);break;
			case 2:rawdata=static_cast<int32_t>(static_cast<uint32_t>(ins[i+1])<<16|ins[i]);break;
			case 4:rawdata=static_cast<int64_t>(static_cast<uint64_t>(ins[i+3])<<48
				|static_cast<uint64_t>(ins[i+2])<<32|static_cast<uint64_t>(ins[i+1])<<16|ins[i]);break;
			}
			switch(ins_info.args[i].type)
			{
			case argument_t::TP_INT:VALUE(value_cell_ref,rawdata,len);break;
			case argument_t::TP_FIXED:VALUE(value_cell_ref,rawdata/FIXED_POINT_RATE,len);break;
			}
		}
		else if(ins_info.args[i].type==argument_t::TP_STR)
		{
			std::string tag_str;
			auto escape_out=[&tag_str](char ch,bool ntbs)
			{
				if(ntbs&&ch=='\0')return false;
				switch(ch)
				{
				case '\t':tag_str+="\\t";break;
				case '\n':tag_str+="\\n";break;
				case '\r':tag_str+="\\r";break;
				case '\a':tag_str+="\\a";break;
				case '\b':tag_str+="\\b";break;
				case '\f':tag_str+="\\f";break;
				case '\v':tag_str+="\\v";break;
				case '\\':tag_str+="\\\\";break;
				default:
					if(0<=ch&&ch<' ')
					{
						tag_str+="\\";
						if(ch>=8)tag_str+=ch/8+'0';
						tag_str+=ch%8+'0';
					}
					else
						tag_str+=ch;
				}
				return true;
			};
			{
				//这样可以检查字符串是否真的是空终止的，
				//只有当\0后的字符全部是\0是才认为字符串是NTBS
				int k=len-1;
				while(k>=0&&ins[i+k]==0)k--;
				for(int j=0;j<=k;++j)
				{
					escape_out(static_cast<char>(ins[i+j]),false);
					escape_out(static_cast<char>(ins[i+j]>>8),j==k);
				}
			}
			VALUE(value_cell_ref,utf8::replace_invalid(tag_str),len);
		}
		else if(ins_info.args[i].type==argument_t::TP_STRW)
		{
			std::u16string tag_str;
			auto escape_out=[&tag_str](char16_t ch,bool ntbs)
			{
				if(ntbs&&ch==0)return false;
				switch(ch)
				{
				case '\t':tag_str+=u"\\t";break;
				case '\n':tag_str+=u"\\n";break;
				case '\r':tag_str+=u"\\r";break;
				case '\a':tag_str+=u"\\a";break;
				case '\b':tag_str+=u"\\b";break;
				case '\f':tag_str+=u"\\f";break;
				case '\v':tag_str+=u"\\v";break;
				case '\\':tag_str+=u"\\\\";break;
				default:
					if(0<=ch&&ch<' ')
					{
						tag_str+=u"\\";
						if(ch>=8)tag_str+=ch/8+u'0';
						tag_str+=ch%8+u'0';
					}
					else
						tag_str+=ch;
				}
				return true;
			};
			{
				int k=len-1;
				while(k>=0&&ins[i+k]==0)k--;
				for(int j=0;j<=k;++j)
					escape_out(static_cast<char16_t>(ins[i+j]),false);
			}
			VALUE(value_cell_ref,utf8::utf16to8(tag_str),len);
		}
		//参数的注释信息
		VALUE(comment_cell_ref,ins_info.args[i].description+((style.comment&PXLSX_COMMENT_COLON)?":":""),len);
		ws.cell(comment_cell_ref).fill(xlnt::fill::solid(xlnt::rgb_color(192u,192u,192u)));
#undef VALUE
	}
}

//根据列的内容，计算合适的列宽度
/*static double calculate_width(const xlnt::worksheet &ws,xlnt::column_t col,double min_width,double muti,const merged_cells_t &merged_cells)
{
	auto n=ws.highest_row();
	
//	std::cout<<"calculate_width"<<std::endl;
	for(xlnt::row_t i=1;i<=n;++i)
	{
		if(ws.has_cell(xlnt::cell_reference(col,i)))
		{
			if(auto it=merged_cells.second.find({col.index,i});it!=merged_cells.second.end())
			{
				auto top_left_index=it->second;
				auto cells_cnt=merged_cells.first.at(top_left_index);
				auto weight=muti/cells_cnt.first;
				min_width=std::max(min_width,get_text_display_size(ws.cell(top_left_index.first,top_left_index.second).to_string())*weight);
			}
			else
				min_width=std::max(min_width,get_text_display_size(ws.cell(col,i).to_string())*muti);
		}
	}
	return min_width;
}*/

void program_t::save_xlsx(const std::filesystem::path &path,const program_xlsx_style &style,std::ostream *error_stream)const
{
//	std::shared_lock _(instr_set_mutex);
	throw_assert(instr_set,std::runtime_error,"尚未加载指令集.");
	xlnt::workbook workbook;
	//属性
	workbook.core_property(xlnt::core_property::creator,"program_t::save_xlsx");
	workbook.extended_property(xlnt::extended_property::application,"ycpies");
	workbook.extended_property(xlnt::extended_property::app_version,YCPIES_VERSION);
	if(style.comment&PXLSX_COMMENT_LEFT)
		workbook.custom_property("style_comment_left","left");

	auto worksheet=workbook.active_sheet();
	const auto instr_len=instr_set->one_instruction_length();
	if(!bin.size())return;
	
	colums_width_t colums_width(instr_len*2+1,-style.width);
	//表头
	worksheet.cell("A1").value("指令");
	worksheet.merge_cells("A1:B1");
	worksheet.column_properties(1).width=7.5;
	for(size_t i=1;i<instr_len;++i)
	{
		worksheet.cell(i*2+1,1).value("MW"+std::to_string(i));
		worksheet.merge_cells(xlnt::range_reference(i*2+1,1,i*2+2,1));
	}
	worksheet.freeze_panes("C2");
	
	for(size_t i=0;i<bin.size();++i)
	{
		try
		{
			if(style.width<0)
				save_xlsx_one_instr<true>(worksheet,style,i,error_stream,&colums_width);
			else
				save_xlsx_one_instr<false>(worksheet,style,i,error_stream,&colums_width);
		}
		catch(...){}//TODO: 处理错误
	}
	//设置列宽度
//	std::cout<<instr_len<<std::endl;
	for(size_t i=1;i<=instr_len*2;++i)
	{
//		std::cout<<style.width<<std::endl;
		worksheet.column_properties(i).width=style.width<0?colums_width[i]*style.width_auto_multi:style.width;
	}
	
//	_.unlock();
	
	std::ofstream save_stream(path,std::ios::binary);
	if(!save_stream)
		throw std::runtime_error("无法打开文件 "+path.generic_u8string());
	workbook.save(save_stream);
}

void program_t::load_xlsx_one_instr(xlnt::worksheet &ws,bool is_comment_left,size_t ssss,std::vector<uint16_t> &ins,std::ostream *error_stream)const
{
	xlnt::row_t row(ssss+2);
	ins.resize(instr_set->one_instruction_length(),0);
	ins[0]=instr_set->instruction_ID(ws.cell(is_comment_left?2:1,row).to_string());
	auto &ins_info=instr_set->at(ins[0]);
	for(size_t i=1;i<ins.size();++i)
	{
		auto len=ins_info.args[i].length;
		if(len==0)continue;
		if(len==-1)
		{
			auto value_cell_ref=xlnt::cell_reference(is_comment_left?i*2+2:i*2+1,row);
			if(ws.cell(value_cell_ref).has_value())
			{
				try{ins[i]=static_cast<uint16_t>(ws.cell(value_cell_ref).value<unsigned>());}
				catch(...){}
				if(error_stream)
					*error_stream<<"警告:\t指令 #"<<ssss+1<<": 指令 "<<ins_info.abbr<<" 的参数 MW"<<i<<" 无意义.\n"<<std::endl;
			}
			continue;
		}
		auto value_cell_ref=xlnt::cell_reference(is_comment_left?i*2+1+len:i*2+1,row);
		if((ins_info.args[i].type&argument_t::MARK_SET)==argument_t::TPS_UNUM||(ins_info.args[i].type&argument_t::MARK_SET)==argument_t::TPS_NUM)
		{
			unsigned long long rawdata;
			if(ins_info.args[i].type==argument_t::TP_FIXED)
				rawdata=static_cast<unsigned long long>(ws.cell(value_cell_ref).value<double>()*FIXED_POINT_RATE+0.5);
			else
			{
				try{rawdata=my_stoull(ws.cell(value_cell_ref).to_string());}
				catch(...)
				{
					if(error_stream)
						*error_stream<<"错误:\t指令 #"<<ssss+1<<": 指令 "<<ins_info.abbr<<" 的参数 MW"<<i<<" 无法被转换为整数.\n"<<std::endl;
				}
			}
			switch(len)
			{
			case 1:ins[i]=static_cast<uint16_t>(rawdata);break;
			case 2:ins[i]=static_cast<uint16_t>(rawdata);ins[i+1]=static_cast<uint16_t>(rawdata>>16);break;
			case 4:ins[i]=static_cast<uint16_t>(rawdata);ins[i+1]=static_cast<uint16_t>(rawdata>>16);
				ins[i+2]=static_cast<uint16_t>(rawdata>>32);ins[i+3]=static_cast<uint16_t>(rawdata>>48);break;
			}
		}
		else if(ins_info.args[i].type==argument_t::TP_STR)
		{
			std::string str_in=ws.cell(value_cell_ref).to_string();
			auto it=str_in.begin();
			auto parse_escape=[&it,&str_in]()->char
			{
				if(it==str_in.end())return '\0';
				if(*it=='\\')
				{
					++it;
					switch(*it)
					{
					case 't':++it;return '\t';
					case 'n':++it;return '\n';
					case 'r':++it;return '\r';
					case 'a':++it;return '\a';
					case 'b':++it;return '\b';
					case 'f':++it;return '\f';
					case 'v':++it;return '\v';
					case '\\':++it;return '\\';
					default:
						if(*it>='0'&&*it<='7')
						{
							char ch=0;
							ch=*it++ -'0';
							if(it!=str_in.end()&&*it>='0'&&*it<='7')
								ch=char(ch*8+ *it++ -'0');
							return ch;
						}
						else
							return '\\';//无效的转义符，不处理
					}
				}
				else
					return *it++;
			};
			for(int j=0;j<len;++j)
			{
				//static_cast<uint16_t>(char) <=> int8 -> int16 -> uint16 —— 当char<0时，高位用1填充
				//而我们需要的是char<0时，高位仍然用0填充，所以需要用int8 -> uint8 -> uint16
				ins[i+j]=static_cast<uint8_t>(parse_escape());
				//此时我们舍弃了char转换成uint16后的高位，所以不关心高位用什么填充
				ins[i+j]|=static_cast<uint16_t>(parse_escape())<<8;
			}
			if(it!=str_in.end()&&error_stream)
				*error_stream<<"警告:\t指令 #"<<ssss+1<<": 字符串 `"<<str_in<<"` 过长，无法放在指令 "<<ins_info.abbr<<" 的参数 MW"<<i<<" 中.\n"<<std::endl;
		}
		else if(ins_info.args[i].type==argument_t::TP_STRW)
		{
			std::u16string str_in=utf8::utf8to16(ws.cell(value_cell_ref).to_string());
			auto it=str_in.begin();
			auto parse_escape=[&it,&str_in]()->char16_t
			{
				if(it==str_in.end())return u'\0';
				if(*it==u'\\')
				{
					++it;
					switch(*it)
					{
					case 't':++it;return u'\t';
					case 'n':++it;return u'\n';
					case 'r':++it;return u'\r';
					case 'a':++it;return u'\a';
					case 'b':++it;return u'\b';
					case 'f':++it;return u'\f';
					case 'v':++it;return u'\v';
					case '\\':++it;return u'\\';
					default:
						if(*it>='0'&&*it<='7')
						{
							char16_t ch=0;
							ch=*it++ -u'0';
							if(it!=str_in.end()&&*it>='0'&&*it<='7')
								ch=char16_t(ch*8+ *it++ -u'0');
							return ch;
						}
						else
							return u'\\';//无效的转义符，不处理
					}
				}
				else
					return *it++;
			};
			for(int j=0;j<len;++j)
			{
				ins[i+j]=static_cast<uint16_t>(parse_escape());
			}
			if(it!=str_in.end()&&error_stream)
				*error_stream<<"警告:\t指令 #"<<ssss+1<<": 字符串 `"<<utf8::utf16to8(str_in)
				<<"` 过长，无法放在指令 "<<ins_info.abbr<<" 的参数 MW"<<i<<" 中.\n"<<std::endl;
		}
	}
}

void program_t::load_xlsx(const std::filesystem::path &path,std::ostream *error_stream)
{
//	std::shared_lock _(instr_set_mutex);
	throw_assert(instr_set,std::runtime_error,"尚未加载指令集.");
	const auto len=instr_set->one_instruction_length();
	xlnt::workbook workbook;
	std::ifstream load_stream(path,std::ios::binary);
	if(!load_stream)
		throw std::runtime_error("无法打开文件 "+path.generic_u8string());
	workbook.load(load_stream);
	const bool is_comment_left=workbook.has_custom_property("style_comment_left");
	auto worksheet=workbook.active_sheet();
	
	std::vector<std::vector<uint16_t>> bin_new;
	bin_new.resize(std::min<size_t>(instr_set->instruction_count(),worksheet.highest_row()-1));
	
	for(size_t i=0;i<bin_new.size();++i)
	{
		try{load_xlsx_one_instr(worksheet,is_comment_left,i,bin_new[i],error_stream);}
		catch(...){}//TODO: 处理错误
	}
	
	bin=std::move(bin_new);
}
