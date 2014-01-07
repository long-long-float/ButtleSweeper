#include"Bitmap.h"

Bitmap::Bitmap() : 
	mCreated(false), mhDC(NULL), mhBmpPrev(NULL)
{
}
Bitmap::Bitmap(HDC hdc, int width, int height) : 
	mCreated(false), mhDC(NULL), mhBmpPrev(NULL)
{
	Create(hdc, width, height);
}
Bitmap::Bitmap(HDC hdc, const char* filename) : 
	mCreated(false), mhDC(NULL), mhBmpPrev(NULL)
{
	Create(hdc, filename);
}

Bitmap::~Bitmap()
{
	if(mCreated)
	{
		HBITMAP hBmp = (HBITMAP)SelectObject(mhDC, mhBmpPrev);
		DeleteObject(hBmp);
		DeleteObject(mhDC);
	}
}

void Bitmap::Create(HDC hdc, int width, int height)
{
	if(mCreated) return;
	
	mhDC = CreateCompatibleDC(hdc);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, width, height);
	mhBmpPrev = (HBITMAP)SelectObject(mhDC, hBmp);
	
	mCreated = true;
}

void Bitmap::Create(HDC hdc, const char* filename)
{
	if(mCreated) return;
	
	mhDC = CreateCompatibleDC(hdc);
	HBITMAP hBmp = (HBITMAP)LoadImageA(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if(!hBmp) return;
	mhBmpPrev = (HBITMAP)SelectObject(mhDC, hBmp);
	
	mCreated = true;
}

