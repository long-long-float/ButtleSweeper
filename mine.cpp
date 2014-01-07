#include<windows.h>
#include<deque>
#include<vector>
#include<list>
#include<cstdlib>
#include<ctime>
#include<string>
#include<sstream>
#define _USE_MATH_DEFINES
#include<math.h>

#include"Bitmap.h"
#include"DebugConsole.h"
#include"ArgumentParser.h"

#define foreach(type, itr, cont) for(type::iterator itr = cont.begin();itr != cont.end();itr++)

#define FlagOn(d, f) ((d) |= (f))
#define FlagOff(d, f) ((d) &= ~(f))

#define UM_DRAW (WM_APP)

#define REAL_WIDTH (gFieldWidth * CELL_SIZE)
#define REAL_HEIGHT (gFieldHeight * CELL_SIZE)

using namespace std;

typedef unsigned char byte;

const TCHAR TITLE[] = TEXT("ButtleSweeper");

const int CELL_SIZE = 16;

const byte MF_BLOCK = 0x01;
const byte MF_BOMB = MF_BLOCK << 1;
const byte MF_NUM = MF_BLOCK << 2;
const byte MF_OBJ = MF_BLOCK << 3;

const int FIELD_BASE_Y = CELL_SIZE * 2;

const int FPS = 30;

const unsigned long TRANS_COLOR = RGB(255, 0, 255);

enum Direction{
	UP = 0,
	RIGHT = 2,
	DOWN = 4,
	LEFT = 6
};

class Point{
public:
	Point() : x(0), y(0){}
	Point(int ax, int ay) : x(ax), y(ay){}
	int x;
	int y;
	
	bool operator==(const Point& p) const{
		return (x == p.x && y == p.y);
	}
};

struct ResourceInfo{
	ResourceInfo(int ax, int ay, int aw, int ah) : x(ax), y(ay), w(aw), h(ah){}
	ResourceInfo() : x(0), y(0), w(0), h(0){}
	int x;
	int y;
	int w;
	int h;
};

class Button{
public:
	
	Button() : pushed(false){
		mRect.left = mRect.top = mRect.right = mRect.bottom = 0;
	}
	
	void setRect(int x, int y, int width, int height){
		mRect.left = x;
		mRect.top = y;
		mRect.right = x + width;
		mRect.bottom = y + height;
	}
	
	bool isInButton(int x, int y){
		return (mRect.left <= x && x <= mRect.right && mRect.top <= y && y <= mRect.bottom);
	}
	
	void push(int x, int y){
		if(isInButton(x, y)){
			pushed = true;
		}
		else{
			pushed = false;
		}
	}
	
	bool pushed;
private:
	RECT mRect;
};

class FallObject{
public:
	FallObject(int ax, int ay, ResourceInfo resInfo)
	 : mx(ax), my(ay), mvx(0), mvy(0), mEnabled(true), mResInfo(resInfo){
		mvx = rand() % 8;
		if(rand() % 2) mvx = -mvx;
		mvy = -8;
	}
	
	int x(){ return mx; }
	int y(){ return my; }
	
	bool enabled(){ return mEnabled; }
	
	void update();
	void draw();
private:
	bool mEnabled;
	
	ResourceInfo mResInfo;
	
	int mx;
	int my;
	int mvx;
	int mvy;
};

enum Type{
	TYPE_DRAGON = 0, 
	TYPE_FLAG,
	TYPE_GUNBATTERY,
	TYPE_BULLET,
	TYPE_RECOVERY,
	TYPE_QUAKER,
};

class Substance{
public:
	Substance(Type aType, Point pos, ResourceInfo resInfo);
	
	virtual ~Substance();
	
	virtual void update() = 0;
	virtual void draw();
	
	virtual void onClick(){
		damage(1);
	}
	
	virtual bool isHit(const Point& pos){
		return (mPos == pos);
	}
	
	const ResourceInfo& getResInfo(){ return mResInfo; }
	
	const Point& getPos(){ return mPos; }
	
	bool isEnabled(){ return mIsEnabled; }
	
	const Type type;
	
	int hp;
	void damage(int val);
	
protected:
	bool mIsEnabled;
	Point mPos;
	ResourceInfo mResInfo;
};

class Dragon : public Substance{
public:
	Dragon(Point pos, unsigned bodySize) : Substance(TYPE_DRAGON, pos, ResourceInfo(CELL_SIZE * 10, CELL_SIZE, CELL_SIZE, CELL_SIZE)), mCount(0), mDire(0){
		for(unsigned i = 0;i < bodySize;i++){
			mQue.push_front(pos);
		}
	}
	~Dragon();
	
	void update();
	void draw();
	
	bool isHit(const Point& pos);
	
private:
	deque<Point> mQue;
	
	int mCount;
	int mDire;
};

class Flag : public Substance{
public:
	Flag(Point pos) : Substance(TYPE_FLAG, pos, ResourceInfo(CELL_SIZE * 11, 0, CELL_SIZE, CELL_SIZE)){}
	~Flag();
	
	void update(){}
	
};

class GunBattery : public Substance{
public:
	GunBattery(Point pos) : Substance(TYPE_GUNBATTERY, pos, ResourceInfo(CELL_SIZE * 10, CELL_SIZE * 2, CELL_SIZE, CELL_SIZE)), mDire(UP), mCount(0){}
	~GunBattery(){}
	
	void update();
	
	void onClick();
	
private:
	Direction mDire;
	int mCount;
};

class Bullet : public Substance{
public:
	Bullet(Point pos, Direction dire, int interval);
	~Bullet(){}
	
	void update();
	
private:
	Direction mDire;
	int mInterval;
	int mCount;
};

class Recovery : public Substance{
public:
	Recovery(Point pos) : Substance(TYPE_RECOVERY, pos, ResourceInfo(CELL_SIZE * 10, CELL_SIZE * 3, CELL_SIZE, CELL_SIZE)), mCount(0){}
	~Recovery(){}
	
	void update();
	
	void onClick();
	
	void recovery();
	
private:
	int mCount;
};

class Quaker : public Substance{
public:
	Quaker(Point pos) : Substance(TYPE_QUAKER, pos, ResourceInfo(CELL_SIZE * 11, CELL_SIZE * 3, CELL_SIZE, CELL_SIZE)), mCount(0){}
	~Quaker(){}
	
	void update();
private:
	int mCount;
};

struct Cell{
	byte num;
	byte flags;
	bool pushed;
};

enum ToolType{
	TOOLTYPE_FLAG = 0,
	TOOLTYPE_GUNBATTERY,
	TOOLTYPE_RECOVERY,
	TOOLTYPE_QUAKER,
	TOOLTYPE_INDEX
};

struct Tool{
	Tool(ToolType aType, ResourceInfo aResInfo, int aHP) : type(aType), resInfo(aResInfo), hp(aHP){}
	ToolType type;
	ResourceInfo resInfo;
	int hp;
};

const Point DIRE8[8] = {
	Point(0, -1),
	Point(1, -1),
	Point(1, 0),
	Point(1, 1),
	Point(0, 1),
	Point(-1, 1),
	Point(-1, 0),
	Point(-1, -1)
};

DebugConsole_r dc;

enum GameState{NORMAL = 0, GAMECLEAR, GAMEOVER} gGameState;

HWND gHWnd = NULL;

unsigned gFieldWidth = 20;
unsigned gFieldHeight = 20;

unsigned gBombNum = 50;

unsigned gTime;
unsigned gFrameNum;

list<FallObject> gFallObjs;

Button gFaceButton;

Bitmap gSrcImg;
Bitmap gBaseImg;

vector<vector<Cell>> gField;

deque<Point> gBreakQue;

list<Substance*> gSubstances;

list<Tool> gTools;

Point gLPushingPos;

bool gIsRPushing;

int gToolNum[TOOLTYPE_INDEX];

bool isEnabledPoint(int x, int y){
	return (0 <= x && x < gFieldWidth && 0 <= y && y < gFieldHeight);
}

bool isInWindow(int x, int y){
	return (0 <= x && x < REAL_WIDTH && 0 <= y && y < REAL_HEIGHT);
}

Substance::Substance(Type aType, Point pos, ResourceInfo resInfo)
 : type(aType), hp(0), mIsEnabled(true), mPos(pos), mResInfo(resInfo){
	FlagOn(gField[mPos.x][mPos.y].flags, MF_OBJ);
}

Substance::~Substance(){
	if(isEnabledPoint(mPos.x, mPos.y)) FlagOff(gField[mPos.x][mPos.y].flags, MF_OBJ);
	gFallObjs.push_back(FallObject(mPos.x * CELL_SIZE, FIELD_BASE_Y + mPos.y * CELL_SIZE, mResInfo));
}

Bullet::Bullet(Point pos, Direction dire, int interval)
 : Substance(TYPE_BULLET, pos, ResourceInfo(CELL_SIZE * 11, CELL_SIZE, CELL_SIZE, CELL_SIZE)), mDire(dire), mInterval(interval), mCount(0){
	FlagOn(gField[mPos.x][mPos.y].flags, MF_OBJ);
}

Dragon::~Dragon(){
	foreach(deque<Point>, itr, mQue){
		FlagOff(gField[itr->x][itr->y].flags, MF_OBJ);
	}
}

Flag::~Flag(){
	//ä“å≥
	gToolNum[TOOLTYPE_FLAG]++;
}

void Substance::damage(int val){
	hp -= val;
	if(hp <= 0){
		hp = 0;
		FlagOff(gField[mPos.x][mPos.y].flags, MF_OBJ);
		mIsEnabled = false;
	}
}

void FallObject::update(){
	if(my > FIELD_BASE_Y + REAL_HEIGHT) mEnabled = false;
	mx += mvx;
	my += mvy;
	mvy += 2;
}

void Dragon::update(){
	mCount++;
	if(mCount == FPS){
		mCount = 0;
		mDire = rand() % 8;
	}
	
	if(mCount % 4 == 0){
		Point frontPos = mQue.front();
		int x = frontPos.x;
		int y = frontPos.y;
		int tx = x + DIRE8[mDire].x;
		int ty = y + DIRE8[mDire].y;
		if(isEnabledPoint(tx, y)) x = tx;
		if(isEnabledPoint(x, ty)) y = ty;
		
		if(mQue.size() >= 2){
			Point oldPos = mQue.at(1);
			if((oldPos.x == x && oldPos.y == y)){
				mDire = rand() % 8;
				return;
			}
		}
		
		bool isHit = false;
		foreach(list<Substance*>, itr, gSubstances){
			if((*itr) == this) continue;
			
			if((*itr)->isHit(Point(x, y))){
				isHit = true;
				if((*itr)->type != TYPE_DRAGON) (*itr)->damage(1);
			}
		}
		if(isHit){
			mDire = rand() % 8;
			return;
		}
		
		mPos.x = x;
		mPos.y = y;
		mQue.push_front(Point(x, y));
		FlagOn(gField[x][y].flags, MF_OBJ);
		for(int i = 0;i < 8;i++){
			int tx = x + DIRE8[i].x, ty = y + DIRE8[i].y;
			if(isEnabledPoint(tx, ty)){
				FlagOn(gField[tx][ty].flags, MF_BLOCK);
			}
		}
		Point backPos = mQue.back();
		mQue.pop_back();
		FlagOff(gField[backPos.x][backPos.y].flags, MF_OBJ);
	}
}

void GunBattery::update(){
	if(mCount == FPS * 5){
		mCount = 0;
		Substance *b = new Bullet(Point(mPos.x + DIRE8[mDire].x, mPos.y + DIRE8[mDire].y), mDire, FPS / 8);
		b->hp = 1;
		gSubstances.push_back(b);
		damage(1);
	}
	mCount++;
}

void Bullet::update(){
	if(mCount == mInterval){
		mCount = 0;
		FlagOff(gField[mPos.x][mPos.y].flags, MF_OBJ);
		mPos.x += DIRE8[mDire].x;
		mPos.y += DIRE8[mDire].y;
		
		if(!isEnabledPoint(mPos.x, mPos.y)){
			mIsEnabled = false;
			return;
		}
		
		FlagOn(gField[mPos.x][mPos.y].flags, MF_OBJ);
		
		foreach(list<Substance*>, itr, gSubstances){
			if((*itr) == this) continue;
			
			if((*itr)->isHit(mPos)){
				(*itr)->damage(1);
				damage(1);
			}
		}
	}
	mCount++;
}

void Recovery::update(){
	if(mCount == FPS * 5){
		mCount = 0;
		recovery();
	}
	mCount++;
}

void Recovery::recovery(){
	foreach(list<Substance*>, itr, gSubstances){
		if((*itr) == this) continue;
		
		if(abs((*itr)->getPos().x - mPos.x) <= 2 && abs((*itr)->getPos().y - mPos.y) <= 2){
			(*itr)->hp += 1;
			damage(1);
		}
		if(!mIsEnabled) break;
	}
}

void breakBlock(int x, int y);
void Quaker::update(){
	if(mCount == FPS * 5){
		mCount = 0;
		for(int cy = 0;cy < 5;cy++){
			for(int cx = 0;cx < 5;cx++){
				if(cx == 3 && cy == 3) continue;
				int x = -2 + mPos.x + cx, y = -2 + mPos.y + cy;
				if(isEnabledPoint(x, y) && !(gField[x][y].flags & MF_BOMB)){
					breakBlock(x, y);
				}
			}
		}
	}
	mCount++;
}

void GunBattery::onClick(){
	Substance::onClick();
	mDire = static_cast<Direction>(mDire + 2);
	if(mDire == 8) mDire = UP;
	
	mResInfo.x = (10 + mDire / 2) * CELL_SIZE;
}

void Recovery::onClick(){
	Substance::onClick();
	recovery();
}

void drawImage(int x, int y, int srcX, int srcY, int srcW, int srcH){
	TransparentBlt(gBaseImg.getHDC(), x, y, srcW, srcH, gSrcImg.getHDC(), srcX, srcY, srcW, srcH, TRANS_COLOR);
}

void drawImage(int x, int y, const ResourceInfo &resInfo){
	drawImage(x, y, resInfo.x, resInfo.y, resInfo.w, resInfo.h);
}

void drawCell(int x, int y, int srcX, int srcY){
	drawImage(x * CELL_SIZE, FIELD_BASE_Y + y * CELL_SIZE, srcX * CELL_SIZE, srcY * CELL_SIZE, CELL_SIZE, CELL_SIZE);
}

void FallObject::draw(){
	drawImage(mx, my, mResInfo);
}

void Substance::draw(){
	if(isEnabledPoint(mPos.x, mPos.y)) drawImage(mPos.x * CELL_SIZE, FIELD_BASE_Y + mPos.y * CELL_SIZE, getResInfo());
}

void Dragon::draw(){
	for(deque<Point>::iterator itr = mQue.begin();itr != mQue.end();itr++){
		drawImage(itr->x * CELL_SIZE, FIELD_BASE_Y + itr->y * CELL_SIZE, mResInfo);
	}
}

bool Dragon::isHit(const Point& pos){
	foreach(deque<Point>, itr, mQue){
		if((*itr) == pos) return true;
	}
	return false;
}

void SetClientRect(HWND hwnd, const RECT *lpRect){
	RECT wndRect, cliRect;
	GetWindowRect(hwnd, &wndRect);
	GetClientRect(hwnd, &cliRect);
	
	MoveWindow(hwnd, lpRect->left, lpRect->top, (lpRect->right - lpRect->left) + (wndRect.right - wndRect.left) - (cliRect.right - cliRect.left), (lpRect->bottom - lpRect->top) + (wndRect.bottom - wndRect.top) - (cliRect.bottom - cliRect.top), FALSE);
}

void AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidth, int nHeight, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, unsigned char alpha){
	BLENDFUNCTION blendFunc = {AC_SRC_OVER, 0, alpha, 0};
	AlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidth, nHeight, hdcSrc, nXOriginSrc, nYOriginSrc, nWidth, nHeight, blendFunc);
}

void unsignedToArray(unsigned num, byte ary[], int arySize){
	for(int i = 0;i < arySize && num != 0;i++){
		ary[i] = num % 10;
		num /= 10;
	}
}

Point getMousePos(HWND hwnd){
	POINT buf;
	GetCursorPos(&buf);
	ScreenToClient(hwnd, &buf);
	return Point(buf.x, buf.y);
}

Point toCellPos(Point pos){
	return Point(pos.x / CELL_SIZE, (pos.y - FIELD_BASE_Y) / CELL_SIZE);
}

void drawNumber(int x, int y, unsigned num, int digit){
	byte *nums = new byte[digit];
	for(int i = 0;i < digit;i++) nums[i] = 0;
	unsignedToArray(num, nums, digit);
	for(int i = 0;i < digit;i++){
		drawImage(x + CELL_SIZE / 2 * (digit - 1 - i), y, CELL_SIZE * 6 + CELL_SIZE / 2 * nums[i], CELL_SIZE * 5, CELL_SIZE / 2, CELL_SIZE / 2);
	}
}

void breakBlock(int x, int y){
	if(gField[x][y].flags & MF_OBJ) return;
	
	if(!(gField[x][y].flags & MF_BLOCK)) return;
	FlagOff(gField[x][y].flags, MF_BLOCK);
	gFallObjs.push_back(FallObject(x * CELL_SIZE, y * CELL_SIZE, ResourceInfo(CELL_SIZE, 0, CELL_SIZE, CELL_SIZE)));
	
	if(gField[x][y].flags & (MF_NUM | MF_BOMB)) return;
	
	gBreakQue.push_back(Point(x, y));
}

void breakBlockAll(){
	for(int y = 0;y < gFieldHeight;y++){
		for(int x = 0;x < gFieldWidth;x++){
			FlagOff(gField[x][y].flags, MF_OBJ);
			breakBlock(x, y);
		}
	}
}

Substance* toolSet(ToolType tool, int x, int y){
	if(!isEnabledPoint(x, y) || !(gField[x][y].flags & MF_BLOCK) || gField[x][y].flags & MF_OBJ) return NULL;
	
	Substance *s = 0;
	switch(tool){
		case TOOLTYPE_FLAG:
			s = new Flag(Point(x, y));
			break;
		case TOOLTYPE_GUNBATTERY:
			s = new GunBattery(Point(x, y));
			break;
		case TOOLTYPE_RECOVERY:
			s = new Recovery(Point(x, y));
			break;
		case TOOLTYPE_QUAKER:
			s = new Quaker(Point(x, y));
			break;
	}
	
	if(s){
		gSubstances.push_back(s);
	}
	return s;
}

void rotateToolMenu(bool isCW){
	if(isCW){
		Tool f = gTools.front();
		gTools.pop_front();
		gTools.push_back(f);
	}
	else{
		Tool b = gTools.back();
		gTools.pop_back();
		gTools.push_front(b);
	}
}

void setup(){
	HDC hdc = GetDC(gHWnd);
	gSrcImg.Create(hdc, "src.bmp");
	ReleaseDC(gHWnd, hdc);
	
	if(!gSrcImg){
		MessageBox(gHWnd, TEXT("src.bmpÇÃì«Ç›çûÇ›Ç…é∏îs"), TEXT("error"), MB_OK);
		SendMessage(gHWnd, WM_CLOSE, 0, 0);
	}
}

void init(){
	
	gGameState = NORMAL;
	
	gField.resize(gFieldWidth);
	for(int i = 0;i < gFieldWidth;i++){
		gField[i].resize(gFieldHeight);
	}
	for(int y = 0;y < gFieldHeight;y++){
		for(int x = 0;x < gFieldWidth;x++){
			gField[x][y].flags = MF_BLOCK;
			gField[x][y].num = 0;
			gField[x][y].pushed = false;
		}
	}
	
	srand((unsigned)time(NULL));
	for(int i = 0;i < gBombNum;i++){
		int x, y;
		do{
			x = rand() % gFieldWidth;
			y = rand() % gFieldHeight;
		}while(gField[x][y].flags & MF_BOMB);
		
		FlagOn(gField[x][y].flags, MF_BOMB);
		FlagOff(gField[x][y].flags, MF_NUM);
		for(int i = 0;i < 8;i++){
			int tx = x + DIRE8[i].x, ty = y + DIRE8[i].y;
			if(!isEnabledPoint(tx, ty)) continue;
			
			gField[tx][ty].num++;
			if(!(gField[tx][ty].flags & MF_BOMB)){
				FlagOn(gField[tx][ty].flags, MF_NUM);
			}
		}
	}
	
	gSubstances.clear();
	
	for(int i = 0;i < 2;i++){
		Substance *d = new Dragon(Point(rand() % gFieldWidth, rand() % gFieldHeight), gFieldWidth * gFieldHeight / 100);
		d->hp = 10;
		gSubstances.push_back(d);
	}
	
	gFallObjs.clear();
	
	HDC hdc = GetDC(gHWnd);
	
	gBaseImg.Create(hdc, REAL_WIDTH, FIELD_BASE_Y + REAL_HEIGHT);
	
	ReleaseDC(gHWnd, hdc);
	
	gFaceButton.setRect(REAL_WIDTH / 2 - CELL_SIZE, 0, CELL_SIZE * 2, CELL_SIZE * 2);
	
	gTime = 0;
	gFrameNum = 0;
	
	gBreakQue.clear();
	
	for(int i = 0;i < TOOLTYPE_INDEX;i++) gToolNum[i] = 0;
	gToolNum[TOOLTYPE_FLAG] = gBombNum;
	gToolNum[TOOLTYPE_GUNBATTERY] = 10;
	gToolNum[TOOLTYPE_RECOVERY] = 5;
	gToolNum[TOOLTYPE_QUAKER] = 10;
	
	gTools.clear();
	
	gTools.push_back(Tool(TOOLTYPE_FLAG, ResourceInfo(CELL_SIZE * 11, 0, CELL_SIZE, CELL_SIZE), 20));
	gTools.push_back(Tool(TOOLTYPE_GUNBATTERY, ResourceInfo(CELL_SIZE * 12, CELL_SIZE, CELL_SIZE, CELL_SIZE), 15));
	gTools.push_back(Tool(TOOLTYPE_RECOVERY, ResourceInfo(CELL_SIZE * 10, CELL_SIZE * 3, CELL_SIZE, CELL_SIZE), 20));
	gTools.push_back(Tool(TOOLTYPE_QUAKER, ResourceInfo(CELL_SIZE * 11, CELL_SIZE * 3, CELL_SIZE, CELL_SIZE), 20));
}

void release(){
	for(list<Substance*>::iterator itr = gSubstances.begin();itr != gSubstances.end();itr++){
		delete *itr;
	}
	gSubstances.clear();
}

void mouseLDown(const Point &mpos, const Point &cpos){
	gFaceButton.push(mpos.x, mpos.y);
	
	if(gGameState != NORMAL) return;
	
	if(isInWindow(mpos.x, mpos.y - FIELD_BASE_Y) && isEnabledPoint(cpos.x, cpos.y)){
		if(!(gField[cpos.x][cpos.y].flags & MF_OBJ)){
			gField[cpos.x][cpos.y].pushed = true;
			gLPushingPos.x = cpos.x;
			gLPushingPos.y = cpos.y;
		}
		else{
			Substance *s = NULL;
			for(list<Substance*>::iterator itr = gSubstances.begin();itr != gSubstances.end();itr++){
				if((*itr)->isHit(cpos)){
					s = *itr;
					break;
				}
			}
			if(s) s->onClick();
		}
	}
}

void mouseLUp(const Point &mpos, const Point &cpos){
	if(gFaceButton.pushed && gFaceButton.isInButton(mpos.x, mpos.y)){
		release();
		init();
	}
	gFaceButton.pushed = false;
	
	if(gGameState != NORMAL) return;
	
	if(isInWindow(mpos.x, mpos.y - FIELD_BASE_Y) && isEnabledPoint(cpos.x, cpos.y) && !(gField[cpos.x][cpos.y].flags & MF_OBJ)){
		if(cpos.x == gLPushingPos.x && cpos.y == gLPushingPos.y){
			breakBlock(cpos.x, cpos.y);
			if(gField[cpos.x][cpos.y].flags & MF_BOMB){
				breakBlockAll();
				gGameState = GAMEOVER;
			}
		}
	}
	
	gField[gLPushingPos.x][gLPushingPos.y].pushed = false;
}

void mouseRDown(const Point &mpos, const Point &cpos){
	if(gGameState != NORMAL) return;
	
	if(gToolNum[TOOLTYPE_FLAG] >= 1){
		gIsRPushing = true;
	}
}

void mouseRUp(const Point &mpos, const Point &cpos){
	if(gGameState != NORMAL) return;
	
	Tool tool = gTools.front();
	if(gToolNum[tool.type] >= 1){
		Substance *t = toolSet(tool.type, cpos.x, cpos.y);
		if(t){
			t->hp = tool.hp;
			gToolNum[tool.type]--;
		}
	}
	gIsRPushing = false;
}

void update(){
	
	for(list<Substance*>::iterator itr = gSubstances.begin();itr != gSubstances.end();){
		if(!(*itr)->isEnabled()){
			delete *itr;
			itr = gSubstances.erase(itr);
			continue;
		}
		itr++;
	}
	
	for(list<FallObject>::iterator itr = gFallObjs.begin();itr != gFallObjs.end();){
		if(!itr->enabled()){
			itr = gFallObjs.erase(itr);
			continue;
		}
		itr++;
	}
	
	static bool preLPressed, preRPressed;
	bool lPressed = (GetAsyncKeyState(VK_LBUTTON) < 0);
	bool rPressed = (GetAsyncKeyState(VK_RBUTTON) < 0);
	
	Point mousePos = getMousePos(gHWnd);
	Point cellPos = toCellPos(mousePos);
	
	if(!preLPressed && lPressed){
		mouseLDown(mousePos, cellPos);
	}
	if(preLPressed && !lPressed){
		mouseLUp(mousePos, cellPos);
	}
	if(!preRPressed && rPressed){
		mouseRDown(mousePos, cellPos);
	}
	if(preRPressed && !rPressed){
		mouseRUp(mousePos, cellPos);
	}
	
	preLPressed = lPressed;
	preRPressed = rPressed;
	
	static BYTE prevKey[256] = {0};
	BYTE key[256];
	
	GetKeyboardState(key);
	if(!(prevKey['Z'] & 0x80) && key['Z'] & 0x80) rotateToolMenu(true);
	else if(!(prevKey['X'] & 0x80) && key['X'] & 0x80) rotateToolMenu(false);
	
	for(int i = 0;i < 256;i++) prevKey[i] = key[i];
	
	if(gGameState == NORMAL){
		foreach(list<Substance*>, itr, gSubstances){
			(*itr)->update();
		}
	}
	
	//ÉuÉçÉbÉNîjâÛ
	for(int i = 0;i < gFieldWidth * gFieldHeight / 100;i++){
		if(!gBreakQue.empty()){
			Point p = gBreakQue.front();
			gBreakQue.pop_front();
			
			for(int i = 0;i < 8;i++){
				int tx = p.x + DIRE8[i].x, ty = p.y + DIRE8[i].y;
				if(isEnabledPoint(tx, ty)){
					breakBlock(tx, ty);
				}
			}
		}
	}
	
	int consistNum = 0;
	for(list<Substance*>::iterator itr = gSubstances.begin();itr != gSubstances.end();itr++){
		if((*itr)->type == TYPE_FLAG && gField[(*itr)->getPos().x][(*itr)->getPos().y].flags & MF_BOMB){
			consistNum++;
		}
	}
	if(consistNum == gBombNum) gGameState = GAMECLEAR;
	
	foreach(list<FallObject>, itr, gFallObjs){
		if(itr->enabled()) itr->update();
	}
	
	//éûä‘
	gFrameNum++;
	if(gFrameNum == FPS){
		gTime++;
		gFrameNum = 0;
	}
}

void drawFrame(){
	HBRUSH hBrush = CreateSolidBrush(GetPixel(gSrcImg.getHDC(), 0, 0));
	SelectObject(gBaseImg.getHDC(), hBrush);
	SelectObject(gBaseImg.getHDC(), GetStockObject(NULL_PEN));
	Rectangle(gBaseImg.getHDC(), 0, 0, REAL_WIDTH + 1, FIELD_BASE_Y + 1);
	DeleteObject(hBrush);
	
	int x = (REAL_WIDTH) / 2 - CELL_SIZE;
	
	int srcX = (3 + gFaceButton.pushed) * CELL_SIZE * 2;
	BitBlt(gBaseImg.getHDC(), x, 0, CELL_SIZE * 2, CELL_SIZE * 2, gSrcImg.getHDC(), srcX, CELL_SIZE, SRCCOPY);
	
	srcX = gGameState * CELL_SIZE * 2;
	drawImage(x, 0, srcX, CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2);
	
	const int len = 4;
	byte nums[len] = {0};
	unsignedToArray(gTime, nums, len);
	for(int i = 0;i < len;i++){
		srcX = nums[i] * CELL_SIZE;
		drawImage(REAL_WIDTH - CELL_SIZE * (i + 1), 0, srcX, CELL_SIZE * 3, CELL_SIZE, CELL_SIZE * 2);
	}
}

void drawField(){
	for(int y = 0;y < gFieldHeight;y++){
		for(int x = 0;x < gFieldWidth;x++){
			Cell cell = gField[x][y];
			//è∞
			BitBlt(gBaseImg.getHDC(), x * CELL_SIZE, FIELD_BASE_Y + y * CELL_SIZE, CELL_SIZE, CELL_SIZE, gSrcImg.getHDC(), 0, 0, SRCCOPY);
			for(int i = 4;i >= 0;i--){
				if(!(cell.flags & 0x01 << i)) continue;
				
				int srcX = i + 1;
				if((0x01 << i) == MF_NUM){
					srcX += (cell.num - 1);
				}
				
				if((0x01 << i) == MF_BLOCK && cell.pushed){
					srcX = 13;
				}
				
				drawCell(x, y, srcX, 0);
			}
		}
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){
	
	switch(msg){
	case WM_CREATE:{
		
		gHWnd = hwnd;
		
		setup();
		init();
		
		return 0;
	}
	case WM_DESTROY:
		release();
		
		PostQuitMessage(0);
		return 0;
	case WM_MOUSEWHEEL:{
		int delta = GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA;
		rotateToolMenu(delta >= 0);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case UM_DRAW:{
		
		if(hwnd != GetForegroundWindow()) return 0;
		
		update();
		
		drawField();
		
		drawFrame();
		
		for(list<Substance*>::iterator itr = gSubstances.begin();itr != gSubstances.end();itr++){
			(*itr)->draw();
		}
		
		Point mousePos = getMousePos(hwnd);
		Point cellPos = toCellPos(mousePos);
		
		if(gIsRPushing){
			ResourceInfo r = gTools.front().resInfo;
			drawCell(cellPos.x, cellPos.y, r.x / CELL_SIZE, r.y / CELL_SIZE);
		}
		
		foreach(list<Substance*>, itr, gSubstances){
			drawNumber((*itr)->getPos().x * CELL_SIZE, FIELD_BASE_Y + (*itr)->getPos().y * CELL_SIZE + CELL_SIZE / 2, (*itr)->hp, 2);
		}
		
		foreach(list<FallObject>, itr, gFallObjs){
			itr->draw();
		}
		
		double angle = 0.0;
		foreach(list<Tool>, itr, gTools){
			ResourceInfo r = itr->resInfo;
			drawImage(mousePos.x + CELL_SIZE * cos(angle + M_PI / 2.0), mousePos.y - CELL_SIZE * sin(angle + M_PI / 2.0), r.x, r.y, r.w, r.h);
			angle += M_PI * 2.0 / gTools.size();
		}
		drawNumber(mousePos.x + CELL_SIZE, mousePos.y - CELL_SIZE, gToolNum[gTools.front().type], 3);
		
		HDC hdc = GetDC(hwnd);
		BitBlt(hdc, 0, 0, REAL_WIDTH, FIELD_BASE_Y + REAL_HEIGHT, gBaseImg.getHDC(), 0, 0, SRCCOPY);
		ReleaseDC(hwnd, hdc);
		
		return 0;
	}
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow){
	
	ArgumentParser arg('/', lpCmdLine);
	string buf;
	if(arg.getParam("/Width", &buf)){
		stringstream ss;
		ss << buf;
		ss >> gFieldWidth;
	}
	if(arg.getParam("/Height", &buf)){
		stringstream ss;
		ss << buf;
		ss >> gFieldHeight;
	}
	if(arg.getParam("/BombNum", &buf)){
		stringstream ss;
		ss << buf;
		ss >> gBombNum;
	}
	
	WNDCLASS winc = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc, 0, 0, hInstance,
		(HICON)LoadImage(NULL, MAKEINTRESOURCE(IDI_APPLICATION), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED),
		(HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED),
		(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL, TITLE
	};
	if(!RegisterClass(&winc)) return -1;
	
	HWND hwnd = CreateWindow(
			TITLE , TITLE ,
			WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
			CW_USEDEFAULT , CW_USEDEFAULT ,
			CW_USEDEFAULT , CW_USEDEFAULT ,
			NULL , NULL , hInstance , NULL
	);
	if(!hwnd) return -1;
	RECT size = {100, 100, 100 + REAL_WIDTH, 100 + FIELD_BASE_Y + REAL_HEIGHT};
	SetClientRect(hwnd, &size);
	ShowWindow(hwnd, nCmdShow);
	
	timeBeginPeriod(1);
	
	MSG msg;
	while(1){
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)){
			if(GetMessage(&msg, NULL, 0, 0) > 0){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else break;
		}
		else{
			DWORD begin = timeGetTime();
			SendMessage(hwnd, UM_DRAW, 0, 0);
			DWORD end = timeGetTime();
			
			int slpTime = 1000 / FPS - (end - begin);
			if(slpTime > 0) Sleep(slpTime);
		}
	}
	
	timeEndPeriod(1);
	
	return msg.wParam;
}