#pragma once

#include<windows.h>

class Bitmap
{
public:
	Bitmap();
	Bitmap(HDC hdc, int width, int height);
	Bitmap(HDC hdc, const char* filename);
	~Bitmap();
	
	operator bool(){ return mCreated; }
	
	void Create(HDC hdc, int width, int height);
	void Create(HDC hdc, const char* filename);
	
	HDC getHDC() const { return mCreated? mhDC : NULL; }
private:
	bool mCreated;
	
	HDC mhDC;
	HBITMAP mhBmpPrev;
};
