//text_width.cpp
//Copyright (C) 2021 张子辰 <zichen350@gmail.com>

// Copying and distribution of this file, with or without modification,
// are permitted in any medium without royalty provided the copyright
// notice and this notice are preserved.  This file is offered as-is,
// without any warranty.

#include "text_width.hpp"
#include <utf8/unchecked.h>

#ifdef YCPIES_DISPLAY_WIDTH_WX
#include <wx/dcscreen.h>
#include <unordered_map>
//#include <shared_mutex>

static std::unordered_map<uint32_t,int> known_width(256);
//static std::shared_mutex know_width_mutex;

double get_text_display_size(const std::string &s)
{
	static double m_width_r=0;
	static wxScreenDC dc;
	if(m_width_r==0)
		m_width_r=1.0/dc.GetTextExtent('M').GetWidth();//大小字母M的宽度的倒数
	int w=0;
	uint32_t ch;
	for(auto i=s.begin();i!=s.end();)
	{
		ch=utf8::unchecked::next(i);
		if(auto j=known_width.find(ch);j!=known_width.end())
			w+=j->second;
		else
		{
			int w2=dc.GetTextExtent(wxUniChar(ch)).GetWidth();
			known_width[ch]=w2;
			w+=w2;
		}
	}
	return w*m_width_r;
	//return 1;
}
#else
double get_text_display_size(const std::string &s)
{
	return s.size();//TODO
}
#endif
