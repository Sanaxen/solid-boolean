/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef COLORSET_H
#define COLORSET_H

#include <vector>

class color_attr_type
{
public:
	inline color_attr_type()
	{
		color = -1;
		attr = 0;
	}
	inline color_attr_type(const int c)
	{
		color = c;
		attr = 0;
	}
	inline ~color_attr_type(){}
	int color;
	unsigned char attr;
};

#define DEFAULT_COLOR	(0xDEDEDE)
class ColorSet
{
	bool default_color;
public:
	color_attr_type default_color_value;
	inline ColorSet():default_color(true),default_color_value(DEFAULT_COLOR){};
	inline ~ColorSet(){};

	inline void __fastcall setDefaultColor()
	{
		default_color = true;
	}
	inline void __fastcall setUserColor()
	{
		default_color = false;
	}
	inline bool __fastcall isDefaultColor() const
	{
		return default_color;
	}

	inline int __fastcall GetSize() const
	{
		return m_pColors.size();
	}

	inline color_attr_type& __fastcall GetColor(const int i)
	{
		if ( default_color ) return default_color_value;
#ifdef _STL_DEBUG
		if(i < 0) return 0;
		if(i >= m_pColors.size()) return 0;
#endif
		return m_pColors[i];
	}

	inline void __fastcall SetColor(const int i, const color_attr_type& vColor)
	{
		if ( default_color ) return;
#ifdef _STL_DEBUG
		if(i < 0) return;
		if(i >= m_pColors.size()) return;
#endif
		m_pColors[i] = vColor;
	}

	inline void __fastcall AddColor(const color_attr_type vColor)
	{
		if ( default_color ) return;
		m_pColors.push_back(vColor);
	}
	inline void __fastcall Add3Color(const color_attr_type vColor[3], const int n)
	{
		if ( default_color ) return;
		m_pColors[n  ] = vColor[0];
		m_pColors[n+1] = vColor[1];
		m_pColors[n+2] = vColor[2];
	}
	inline color_attr_type & __fastcall operator[](const int index)
	{
		if ( default_color )
		{
			color_attr_type& pColor = default_color_value;
			return pColor;
		}

		color_attr_type & pColor = m_pColors[index];

		return pColor;
	}

	inline int __fastcall ToInt(const int r, const int g, const int b)
	{
		int color;
		
		color = r << 16;
		color |= (g << 8);
		color |= b;

		return color;
	}

	inline void __fastcall ToRGB(const int color, int* r, int *g, int* b)
	{
		*r = (color & 0xff0000) >> 16;
		*g = (color & 0x00ff00) >> 8;
		*b = (color & 0x0000ff);
	}

	typedef std::vector<color_attr_type> ColorVector;

	ColorVector m_pColors;

	inline void __fastcall Reserve(const int size) 
	{
		if ( default_color ) return;
		m_pColors.reserve(size);
	}
	inline void __fastcall Resize(const int size) 
	{
		if ( default_color ) return;
		m_pColors.resize(size);
	}
	inline void __fastcall Assgin(ColorSet & vColors)
	{
		m_pColors.assign(vColors.m_pColors.begin(), vColors.m_pColors.end());
	}

	inline void __fastcall Assgin(const int num, int* vColors)
	{
		m_pColors.assign(vColors, vColors+num);
	}
};

#endif // COLORSET_H

