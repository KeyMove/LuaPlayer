#include "stdafx.h"
#include "LoadLua.h"

extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#pragma comment(lib,"Lualib.lib")

union u2i{
	unsigned long l;
	unsigned int li[2];
	unsigned char lc[4];
	unsigned int i;
	unsigned char c[2];
};

#define _16T8H(x) (((union u2i*)&x)->c[0])//提取16位高8位
#define _16T8L(x) (((union u2i*)&x)->c[1])//提取16位低8位

#define _32T16H(x) (((union u2i*)&x)->li[0])//提取32位高16位
#define _32T16L(x) (((union u2i*)&x)->li[1])//提取32位低16位

#define _32T8HH(x) (((union u2i*)&x)->lc[0])//提取32位低8位
#define _32T8H(x) (((union u2i*)&x)->lc[1])//提取32位中低8位
#define _32T8L(x) (((union u2i*)&x)->lc[2])//提取32位中高8位
#define _32T8LL(x) (((union u2i*)&x)->lc[3])//提取32高8位

typedef struct _rgbval
{
	union{
		struct{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
		};
		unsigned long bgra;
	};
}RGBVAL;


HDC m_hdc;
CDC m_cdc;
HWND m_hwnd;
RGBVAL *rgb=nullptr;
CDC tdc;
CBitmap tbm;
int w_w=0,w_h=0;
BITMAP map;
CString printout;

LoadLua::LoadLua(void)
	: ErrorLog(_T(""))
{

}


LoadLua::~LoadLua(void)
{

}

lua_State *luastack;

int mouse_leftdown(lua_State *lu)
{
	mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
	return 0;
}

int mouse_leftup(lua_State *lu)
{
	mouse_event(MOUSEEVENTF_LEFTUP ,0,0,0,0);
	return 0;
}
int mouse_rightdown(lua_State *lu)
{
	mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
	return 0;
}

int mouse_rightup(lua_State *lu)
{
	mouse_event(MOUSEEVENTF_RIGHTUP ,0,0,0,0);
	return 0;
}

int mouse_moveto(lua_State *lu)
{
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE ,x,y,0,0);
	return 0;
}

int mouse_moveaddto(lua_State *lu)
{
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	mouse_event(MOUSEEVENTF_MOVE ,x,y,0,0);
	return 0;
}

int win_topwindowname(lua_State *lu)
{
	CString title;
	WCHAR buff[100];
	char out[100];
	int count=::GetWindowText(GetForegroundWindow(),buff,100);
	WideCharToMultiByte( CP_ACP, 0, (LPWCH)buff, -1, out, 100, NULL, NULL ) ;
	lua_pushstring(lu,out);
	return 1;
}

int win_topwindowpos(lua_State *lu)
{
	RECT size;
	CWnd::FromHandle(GetForegroundWindow())->GetWindowRect(&size);
	lua_pushinteger(lu, size.left);
	lua_pushinteger(lu, size.top);
	lua_pushinteger(lu, size.right-size.left);
	lua_pushinteger(lu, size.bottom-size.top);
	return 4;
}

int win_delay(lua_State *lu)
{
	int time = lua_tointeger(lu, 1);
	Sleep(time);
	return 0;
}

CString ANSI2UNICODE(const char* s)
{
	CString out;
	int count=::MultiByteToWideChar( CP_ACP , 0,s,-1,NULL,0);
	WCHAR *pbuff=new WCHAR[count];
	::MultiByteToWideChar( CP_ACP , 0,s,-1,pbuff,count);
	out.Format(_T("%s"),pbuff);
	delete[] pbuff;
	return out;
}

int msg(lua_State *lu)
{
	const char *s=lua_tostring(lu,-1);
	AfxMessageBox(ANSI2UNICODE(s));
	return 1;
}

int win_getpix(lua_State *lu)
{
	unsigned char r,g,b;
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	unsigned int rgb=m_cdc.GetPixel(x,y);
	r=_32T8L(rgb);
	g=_32T8H(rgb);
	b=_32T8HH(rgb);
	lua_pushnumber(lu,r);
	lua_pushnumber(lu,g);
	lua_pushnumber(lu,b);
	return 3;
}

int win_getpixcolor(lua_State *lu)
{
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	int rgb=m_cdc.GetPixel(x,y);
	lua_pushnumber(lu,rgb);
	return 1;
}

int win_ecolor(lua_State *lu)
{
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	int rgb=lua_tointeger(lu, 3);
	if(m_cdc.GetPixel(x,y)==rgb)
		lua_pushboolean(lu,true);
	else
		lua_pushboolean(lu,false);
	return 1;
}

int win_color(lua_State *lu)
{
	lua_pushnumber(lu,RGB(lua_tointeger(lu, 1),lua_tointeger(lu, 2),lua_tointeger(lu, 3)));
	return 1;
}

int win_setpencolor(lua_State *lu)
{
	int color=lua_tointeger(lu, 1);
	m_cdc.SetDCBrushColor(color);
	m_cdc.SetDCPenColor(color);
	return 0;
}

int win_line(lua_State *lu)
{
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	int ex = lua_tointeger(lu, 3);
    int ey = lua_tointeger(lu, 4);
	m_cdc.MoveTo(x,y);
	m_cdc.LineTo(ex,ey);
	return 0;
}

int win_box(lua_State *lu)
{
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	int ex = lua_tointeger(lu, 3);
    int ey = lua_tointeger(lu, 4);
	m_cdc.MoveTo(x,y);
	m_cdc.LineTo(ex,y);
	m_cdc.LineTo(ex,ey);
	m_cdc.LineTo(x,ey);
	m_cdc.LineTo(x,y);
	return 0;
}

int win_textout(lua_State *lu)
{
	int x = lua_tointeger(lu, 1);
    int y = lua_tointeger(lu, 2);
	const char *s=lua_tostring(lu,3);
	m_cdc.TextOutW(x,y,ANSI2UNICODE(s));
	return 0;
}


int bm_cont(lua_State *lu)
{
	int type = lua_tointeger(lu, 1);
	static int sx,sy,ex,ey;
	static int px,py;
	int lx,ly;
	long index;
	unsigned long color;
	int len;
	int i,count;
    switch(type)
	{
		case 1:
			sx = lua_tointeger(lu, 2);
			sy = lua_tointeger(lu, 3);
			ex = lua_tointeger(lu, 4);
			ey = lua_tointeger(lu, 5);
			if(sx<0)sx=0;
			if(sy<0)sy=0;
			if(ex>map.bmWidth)ex=map.bmWidth;
			if(ey>map.bmHeight)ey=map.bmHeight;
			tdc.BitBlt(0,0,ex-sx,ey-sy,&m_cdc,sx,sy,SRCCOPY);
			len=map.bmWidthBytes*map.bmHeight;
			tbm.GetBitmapBits(len,rgb);
			py=px=0;
			break;
		case 2:
			color = lua_tointeger(lu, 2);
			index=0;
			if(px)
			{
				if(!(++px<ex-sx))
				{
					px=0;
					if((py+1)<(ey-sy))
						py++;
				}
			}
			for(int y=py;y<ey-sy;y++)
			{
				index=y*map.bmWidth;
				for(int x=px;x<ex-sx;x++)
				{
					if(color==(rgb[index+x].bgra&0x00ffffff))
					{
						px=x;
						py=y;
						lua_pushboolean(lu,true);
						lua_pushnumber(lu,x);
						lua_pushnumber(lu,y);
						return 3;
					}
				}
				px=0;
			}
			px=0;
			py=0;
			break;
		case 3:
			lx = lua_tointeger(lu, 2);
			ly = lua_tointeger(lu, 3);
			color = lua_tointeger(lu, 4);
			if((px+lx)<0)break;
			if((py+ly)<0)break;
			if((px+lx)>(ex-sx))break;
			if((py+ly)>(ey-sy))break;
			index=(py+ly)*map.bmWidth+px+lx;
			if((rgb[index].bgra&0x00ffffff)==color)
			{
				lua_pushboolean(lu,true);
				return 1;
			}
			break;
		case 4:
			delete[] rgb;
			tdc.DeleteDC();
			tbm.DeleteObject();
			break;
		case 5:
			lx = lua_tointeger(lu, 2);
			ly = lua_tointeger(lu, 3);
			if(lx<0)break;
			if(ly<0)break;
			if(lx>(ex-sx))break;
			if(ly>(ey-sy))break;
			index=(ly)*map.bmWidth+lx;
			lua_pushinteger(lu,rgb[index].r);
			lua_pushinteger(lu,rgb[index].g);
			lua_pushinteger(lu,rgb[index].b);
			return 3;
		case 6:
			tdc.BitBlt(0,0,ex-sx,ey-sy,&m_cdc,sx,sy,SRCCOPY);
			len=map.bmWidthBytes*map.bmHeight;
			tbm.GetBitmapBits(len,rgb);
			return 0;
		case 7:
			lx = lua_tointeger(lu, 2);
			ly = lua_tointeger(lu, 3);
			color = lua_tointeger(lu, 4);
			if(lx<0)break;
			if(ly<0)break;
			if(lx>(ex-sx))break;
			if(ly>(ey-sy))break;
			index=(ly)*map.bmWidth+lx;
			if(color==(rgb[index].bgra&0x00ffffff))
			{
				lua_pushboolean(lu,true);
				return 1;
			}
			break;
		case 8:
			count = lua_tointeger(lu, 2);
			i=3;
			while(count--)
			{
				lx = lua_tointeger(lu, i);
				ly = lua_tointeger(lu, i+1);
				color = lua_tointeger(lu, i+2);
				i+=3;
				if((px+lx)<0)break;
				if((py+ly)<0)break;
				if((px+lx)>(ex-sx))break;
				if((py+ly)>(ey-sy))break;
				index=(py+ly)*map.bmWidth+px+lx;
				if((rgb[index].bgra&0x00ffffff)!=color)
				{
					lua_pushboolean(lu,false);
					return 1;
				}
			}
			lua_pushboolean(lu,true);
			return 1;
		case 9:
			count = lua_tointeger(lu, 2);
			i=3;
			while(count--)
			{
				lx = lua_tointeger(lu, i);
				ly = lua_tointeger(lu, i+1);
				color = lua_tointeger(lu, i+2);
				i+=3;
				if(lx<0)break;
				if(ly<0)break;
				if(lx>(ex-sx))break;
				if(ly>(ey-sy))break;
				index=(ly)*map.bmWidth+lx;
				if((rgb[index].bgra&0x00ffffff)!=color)
				{
					lua_pushboolean(lu,false);
					return 1;
				}
			}
			lua_pushboolean(lu,true);
			return 1;
	}
	lua_pushboolean(lu,false);
	return 1;
}

int bm_bgr(lua_State *lu)
{
	RGBVAL bgr;
	bgr.r=(unsigned char)lua_tonumber(lu,1);
	bgr.g=(unsigned char)lua_tonumber(lu,2);
	bgr.b=(unsigned char)lua_tonumber(lu,3);
	bgr.a=0;
	lua_pushnumber(lu,bgr.bgra);
	return 1;
}

int win_settime(lua_State *lu)
{
	int id = lua_tointeger(lu, 1);
    int time = lua_tointeger(lu, 2);
	SetTimer(m_hwnd,id,time,NULL);
	return 0;
}

int win_killtime(lua_State *lu)
{
	int id = lua_tointeger(lu, 1);
	KillTimer(m_hwnd,id);
	return 0;
}

int win_getbackdc(lua_State *lu)
{
	CString str;
	str=ANSI2UNICODE(lua_tostring(lu,1));
	HWND thwnd=FindWindow(NULL,str);
	if(thwnd==NULL)
	{
		return 0;
	}
	HDC thdc=GetDC(thwnd);
	CDC tcdc;
	tcdc.Attach(thdc);
	m_cdc.BitBlt(0,0,320,320,&tcdc,0,0,SRCCOPY);
	return 0;
}

int win_sendbackdcmessage(lua_State *lu)
{
	CString str;
	Beep(1500,1000);
	return 0;
}

int reprint(lua_State *lu)
{
	printout+=ANSI2UNICODE(lua_tostring(lu,1));
	printout+=_T("\r\n");
	return 0;
}

const char *timercode="TimerList={}\r\nfunction Timer(i,t,f)\r\ni=i+0\r\nt=t+1\r\nif f==nil or TimerList[i]~=nil then\r\nreturn\r\nend\r\nTimerList[i]={[1]=i,[2]=f}\r\nsys_settime(i,t)\r\nend\r\n\r\nfunction TimerClose(i)\r\nif TimerList[i] then\r\nsys_killtime(i)\r\nTimerList[i]=nil\r\nend\r\nend\r\n\r\nfunction TimerCall(index)\r\nif TimerList[index] then\r\nTimerList[index][2]()\r\nend\r\nend\r\n\r\nfunction TimerCloseAll()\r\nfor i=1,#TimerList do\r\nif TimerList[i] then\r\nsys_killtime(i)\r\nTimerList[i]=nil\r\nend\r\nend\r\nend";
bool LoadLua::InitLua(void)
{
	luastack=luaL_newstate();
	luaL_openlibs(luastack);
	lua_register(luastack,"message",msg);
	lua_register(luastack,"sys_settime",win_settime);
	lua_register(luastack,"sys_killtime",win_killtime);
	lua_register(luastack,"print",reprint);
	luaL_dostring(luastack,timercode);
	windowapi();
	return false;
}


bool LoadLua::doFile(CString file)
{
	char buff[100];
	char *str=(char*)(LPCTSTR)file;
	int i;
	for(i=0;i<file.GetLength();i++)
	{
		buff[i]=str[i*2];
	}
	buff[i]=0;
	if(luaL_loadfile(luastack,buff))
	{
		//AfxMessageBox(_T("加载错误"));
		ErrorLog+=ANSI2UNICODE(lua_tostring(luastack,-1));
		ErrorLog+=_T("\r\n");
		return true;
	}
	if(lua_pcall(luastack,0,0,0))
	{
		//AfxMessageBox(_T("执行错误"));
		//msg(luastack);
		ErrorLog+=ANSI2UNICODE(lua_tostring(luastack,-1));
		ErrorLog+=_T("\r\n");
		return true;
	}
	return false;
}

void LoadLua::windowapi(void)
{
	lua_register(luastack,"w_getpix",win_getpix);
	lua_register(luastack,"w_getpixcolor",win_getpixcolor);
	lua_register(luastack,"w_checkcolor",win_ecolor);
	lua_register(luastack,"w_color",win_color);
	lua_register(luastack,"w_delay",win_delay);
	lua_register(luastack,"w_gettopwindowname",win_topwindowname);
	lua_register(luastack,"w_gettopwindowpos",win_topwindowpos);
	lua_register(luastack,"w_drawline",win_line);
	lua_register(luastack,"w_drawbox",win_box);
	lua_register(luastack,"w_setpencolor",win_setpencolor);
	lua_register(luastack,"w_textout",win_textout);
	lua_register(luastack,"w_mouseld",mouse_leftdown);
	lua_register(luastack,"w_mouselu",mouse_leftup);
	lua_register(luastack,"w_mouserd",mouse_rightdown);
	lua_register(luastack,"w_mouseru",mouse_rightup);
	lua_register(luastack,"w_mousemoveto",mouse_moveto);
	lua_register(luastack,"w_mousemoveat",mouse_moveaddto);
	lua_register(luastack,"w_bitmap",bm_cont);
	lua_register(luastack,"w_bitmapcolor",bm_bgr);
	lua_register(luastack,"w_getbkdc",win_getbackdc);
	lua_register(luastack,"w_sendbkdc",win_sendbackdcmessage);
}

void LoadLua::TimerCall(int index)
{
	static int state;
	if(!state)
	{
		state=1;
		lua_getglobal(luastack,"TimerCall");
		lua_pushnumber(luastack,index);
		if(lua_pcall(luastack,1,0,0))
		{
			ErrorLog+=ANSI2UNICODE(lua_tostring(luastack,-1));
			ErrorLog+=_T("\r\n");
		}
		state=0;
	}
}


void LoadLua::SetHWND(HWND hwnd)
{
	m_hwnd=hwnd;
	m_hdc=GetDC(NULL);
	m_cdc.Attach(m_hdc);
	m_cdc.SelectObject(GetStockObject(DC_PEN));
	m_cdc.GetCurrentBitmap()->GetBitmap(&map);
	rgb=new RGBVAL[map.bmWidth*map.bmHeight];
	tdc.CreateCompatibleDC(NULL);			
	tbm.CreateCompatibleBitmap(&m_cdc,map.bmWidth,map.bmHeight);
	tdc.SelectObject(&tbm);
	ErrorLog=_T("");
}


int LoadLua::CloseAllTimer(void)
{
	lua_getglobal(luastack,"TimerCloseAll");
	if(lua_pcall(luastack,0,0,0))
	{
		ErrorLog+=ANSI2UNICODE(lua_tostring(luastack,-1));
		ErrorLog+=_T("\r\n");
	}
	lua_close(luastack);
	InitLua();
	return 0;
}


CString LoadLua::GetErrorLog(void)
{
	CString cc;
	cc=ErrorLog;
	cc+=printout;
	printout=_T("");
	ErrorLog=_T("");
	return cc;
}

