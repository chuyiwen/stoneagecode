#include "npc_lua.h"
#include "util.h"
#include "char.h"
#include "char_base.h"
#include "char_data.h"
#include "anim_tbl.h"
#include "object.h"
#include "battle.h"
#include "npcutil.h"
#include "item.h"
#include "readmap.h"

#ifdef _JZ_NEWSCRIPT_LUA

typedef enum
{
	LUAITEM_PREOVERFUNC = ITEM_PREOVERFUNC - ITEM_FIRSTFUNCTION,
	LUAITEM_POSTOVERFUNC = ITEM_POSTOVERFUNC - ITEM_FIRSTFUNCTION,
	LUAITEM_WATCHFUNC = ITEM_WATCHFUNC - ITEM_FIRSTFUNCTION,
	LUAITEM_USEFUNC = ITEM_USEFUNC - ITEM_FIRSTFUNCTION,
	LUAITEM_ATTACHFUNC = ITEM_ATTACHFUNC - ITEM_FIRSTFUNCTION,
	LUAITEM_DETACHFUNC = ITEM_DETACHFUNC - ITEM_FIRSTFUNCTION,
	LUAITEM_DROPFUNC = ITEM_DROPFUNC - ITEM_FIRSTFUNCTION,
	LUAITEM_PICKUPFUNC = ITEM_PICKUPFUNC - ITEM_FIRSTFUNCTION,
#ifdef _Item_ReLifeAct
	LUAITEM_DIERELIFEFUNC = ITEM_DIERELIFEFUNC - ITEM_FIRSTFUNCTION,
#endif
}LUAITEM_FUNC;

lua_State *M_Script_Lua = NULL;	//实例句柄
unsigned int M_Create_Num = 0;
char M_OutErrMsg[1024];

extern lua_const M_Lua_RegConstList[];

static luaL_Reg M_RegList[] = {
	{NULL, NULL},
};

static luaL_Reg NL_RegList[] = {
//NL
	{"GetErrorStr", NPC_Lua_NL_GetErrorStr},
	{"CreateNpc", NPC_Lua_NL_CreateNpc},
	{"DelNpc", NPC_Lua_NL_DelNpc},
	{NULL, NULL},
};

static luaL_Reg Char_RegList[] = {
//获取数据的接口
	{"IsEventEnd", NPC_Lua_Char_IsEventEnd},
	{"IsEventNow", NPC_Lua_Char_IsEventNow},

	{"FindItemId", NPC_Lua_Char_FindItemId},
	{"FindPetEnemyId", NPC_Lua_Char_FindPetEnemyId},

	{"GetData", NPC_Lua_Char_GetData},
	{"GetItemId", NPC_Lua_Char_GetItemId},
	{"GetPetEnemyId", NPC_Lua_Char_GetPetEnemyId},
	{"GetItemIndex", NPC_Lua_Char_GetItemIndex},
	{"GetPetIndex", NPC_Lua_Char_GetPetIndex},
	{"GetTeamIndex", NPC_Lua_Char_GetTeamIndex},

//设置数据的接口
	{"ClrEvtEnd", NPC_Lua_Char_ClrEvtEnd},
	{"ClrEvtNow", NPC_Lua_Char_ClrEvtNow},
	{"ClrClrEvt", NPC_Lua_Char_ClrEvt},
	{"SetEvtEnd", NPC_Lua_Char_SetEvtEnd},
	{"SetEvtNow", NPC_Lua_Char_SetEvtNow},
	{"SetData", NPC_Lua_Char_SetData},

//事件设置的接口
	{"SetWalkPreEvent", NPC_Lua_Char_SetWalkPreEvent},
	{"SetWalkPostEvent", NPC_Lua_Char_SetWalkPostEvent},
	{"SetPreOverEvent", NPC_Lua_Char_SetPreOverEvent},
	{"SetPostOverEvent", NPC_Lua_Char_SetPostOverEvent},
	{"SetWatchEvent", NPC_Lua_Char_SetWatchEvent},
	{"SetLoopEvent", NPC_Lua_Char_SetLoopEvent},
	{"SetTalkedEvent", NPC_Lua_Char_SetTalkedEvent},
	{"SetOFFEvent", NPC_Lua_Char_SetOFFEvent},
	{"SetLookedEvent", NPC_Lua_Char_SetLookedEvent},
	{"SetItemPutEvent", NPC_Lua_Char_SetItemPutEvent},
	{"SetWindowTalkedEvent", NPC_Lua_Char_SetWindowTalkedEvent},
#ifdef _USER_CHARLOOPS
	{"SetCharLoopsEvent", NPC_Lua_Char_SetCharLoopsEvent},
	{"SetBattleProPertyEvent", NPC_Lua_Char_SetBattleProPertyEvent},
#endif
//会员点接口
	{"VipPoint", NPC_Lua_Char_VipPoint},
	{"HealAll", NPC_Lua_Char_HealAll},
	{"GetPetSkillId", NPC_Lua_Char_GetPetSkillId},
	{"GetPetSkillName", NPC_Lua_Char_GetPetSkillName},
	{"GetPetSkillMsg", NPC_Lua_Char_GetPetSkillMsg},
	{"SetPetSkill", NPC_Lua_Char_SetPetSkill},
	{NULL, NULL},
};

static luaL_Reg Item_RegList[] = {
//获取数据的接口
	{"GetData", NPC_Lua_Item_GetData},

//设置数据的接口
	{"SetData", NPC_Lua_Item_SetData},

//事件设置的接口
	{"SetPreOverEvent", NPC_Lua_Item_SetPreOverEvent},
	{"SetPostOverEvent", NPC_Lua_Item_SetPostOverEvent},
	{"SetWatchEvent", NPC_Lua_Item_SetWatchEvent},
	{"SetUseEvent", NPC_Lua_Item_SetUseEvent},
	{"SetAttachEvent", NPC_Lua_Item_SetAttachEvent},
	{"SetDetachEvent", NPC_Lua_Item_SetDetachEvent},
	{"SetDropEvent", NPC_Lua_Item_SetDropEvent},
	{"SetPickUPEvent", NPC_Lua_Item_SetPickUPEvent},
#ifdef _Item_ReLifeAct
	{"SetDieReLifeEvent", NPC_Lua_Item_SetDieReLifeEvent},
#endif
	{NULL, NULL},
};

static luaL_Reg Obj_RegList[] = {
//获取数据的接口
	{"GetType", NPC_Lua_Obj_GetType},
	{"GetCharType", NPC_Lua_Obj_GetCharType},
	{"GetCharIndex", NPC_Lua_Obj_GetCharIndex},
	{"GetX", NPC_Lua_Obj_GetX},
	{"GetY", NPC_Lua_Obj_GetY},
	{"GetFloor", NPC_Lua_Obj_GetFloor},

//事件设置的接口
	{"SetType", NPC_Lua_Obj_SetType},
	{"SetCharType", NPC_Lua_Obj_SetCharType},
	{"SetX", NPC_Lua_Obj_SetX},
	{"SetY", NPC_Lua_Obj_SetY},
	{"SetFloor", NPC_Lua_Obj_SetFloor},
	{NULL, NULL},
};

static luaL_Reg Battle_RegList[] = {
//获取数据的接口
	{"GetPlayIndex", NPC_Lua_Battle_GetPlayIndex},

//设置数据的接口
	{"SetNORisk", NPC_Lua_Battle_SetNORisk},
	{"SetMod", NPC_Lua_Battle_SetMod},
	{"SetType", NPC_Lua_Battle_SetType},

//事件设置的接口
	{"SetWinEvent", NPC_Lua_Battle_SetWinEvent},
	{NULL, NULL},
};

static luaL_Reg NLG_RegList[] = {
//功能接口
	{"CheckInFront", NPC_Lua_NLG_CheckInFront},
	{"CheckObj", NPC_Lua_NLG_CheckObj},
	{"CharLook", NPC_Lua_NLG_CharLook},
	{"CreateBattle", NPC_Lua_NLG_CreateBattle},
	{"CreateBattlePvP", NPC_Lua_NLG_CreateBattlePvP},
	{"SearchWatchBattleRandIndex", NPC_Lua_NLG_SearchWatchBattleRandIndex},

	{"DelPet", NPC_Lua_NLG_DelPet},
	{"DelHaveIndexPet", NPC_Lua_NLG_DelHaveIndexPet},
	{"DelItem", NPC_Lua_NLG_DelItem},
	{"DischargeParty", NPC_Lua_NLG_DischargeParty},
	
	{"GivePet", NPC_Lua_NLG_GivePet},
	{"GiveItem", NPC_Lua_NLG_GiveItem},
	{"GiveOneItem", NPC_Lua_NLG_GiveOneItem},
	{"GiveRandItem", NPC_Lua_NLG_GiveRandItem},
	{"GetOnLinePlayer", NPC_Lua_Char_GetOnLinePlayer},

	{"ShowWindowTalked", NPC_Lua_NLG_ShowWindowTalked},
	{"SetAction", NPC_Lua_NLG_SetAction},

	{"TalkToCli", NPC_Lua_NLG_TalkToCli},
	{"TalkToFloor", NPC_Lua_NLG_TalkToFloor},

	{"UpChar", NPC_Lua_NLG_UpChar},
	{"UpStateBySecond", NPC_Lua_NLG_UpStateBySecond}, 
	{"UpStateByThird", NPC_Lua_NLG_UpStateByThird},
	{"Update_Party", NPC_Lua_NLG_Update_Party},

	{"Warp", NPC_Lua_NLG_Warp},
	{"WalkMove", NPC_Lua_NLG_WalkMove},
	{"WatchEntry", NPC_Lua_NLG_WatchEntry},
	
	{"GetMaxPlayNum", NPC_Lua_NLG_GetMaxPlayNum},
	{"CheckPlayIndex", NPC_Lua_NLG_CheckPlayIndex},
	
	{"Save", NPC_Lua_NLG_Save},

	{NULL, NULL},
};

static luaL_Reg M_NTIntRegList[] = {
	{"AINew", NPC_Lua_NTInt_New},
	{"AIGet", NPC_Lua_NTInt_Get},
	{"AISet", NPC_Lua_NTInt_Set},
	{"AILen", NPC_Lua_NTInt_Len},
	{NULL, NULL},
};

static SCRIPTREGLIB M_NTInt_Lib[] = {
	{"NTInt", &M_NTIntRegList},
	{NULL, NULL},
};

static SCRIPTREGLIB M_RegLib[] = {
	{"NL", &NL_RegList},
	{"NLG", &NLG_RegList},
	{"Char", &Char_RegList},
	{"Item", &Item_RegList},
	{"Obj", &Obj_RegList},
	{"Battle", &Battle_RegList},
	{NULL, NULL},
};

static SCRIPTREGCLASS M_RegClass[] = {
	{NULL, NULL, NULL},
};

static char *Array_INL[] = {
	"__index", "__newindex", "__len", NULL
};

static char *Array_GSL[] = {
	"AIGet", "AISet", "AILen", NULL
};

static SCRIPTREGARRAY M_RegArray[] = {
	{
		"Array.NTInt",
		&Array_INL[0],
		&Array_GSL[0],
		&M_NTInt_Lib,
	},
	{
		NULL,
		{NULL},
		{NULL},
		NULL,
	},
};

int NPC_Lua_Init(const char *_DoFile)
{
	int TM_Ret = 0;

	M_Script_Lua = lua_open();
  if (M_Script_Lua == NULL) {
    print("LUAInit: cannot create state: not enough memory\n");
    return -1;
  }
  M_OutErrMsg[0] = '\0';

  //停止垃圾收集
  lua_gc(M_Script_Lua, LUA_GCSTOP, 0);

  luaL_openlibs(M_Script_Lua);

  lua_gc(M_Script_Lua, LUA_GCRESTART, 0);
	
	//设置内置常量
	lua_setconstlist(M_Script_Lua, M_Lua_RegConstList);

	//设置Lua引擎扩展
	print("LUA RegFuncNum:%d\n", NPC_Lua_RegCallEx(M_Script_Lua, M_RegList));
	print("LUA RegLibNum:%d\n", NPC_Lua_RegLibEx(M_Script_Lua, M_RegLib));
	print("LUA RegClassNum:%d\n", NPC_Lua_RegClassEx(M_Script_Lua, M_RegClass));
	print("LUA RegArrayNum:%d\n", NPC_Lua_RegArrayEx(M_Script_Lua, M_RegArray));

	print("LNS引擎初始化完成 引擎版本: 1.1.26\n");
	//删除之前LUA建立的NPC
	int i;
	int charnum = getFdnum()+ getPetcharnum()+getOtherscharnum();
	for(i=0;i<charnum;i++){
		if(!CHAR_CHECKINDEX(i)) continue;
		if(CHAR_getWorkInt(i,CHAR_WORKNPCTYPE)==2){
			NPC_Lua_Del(i);
		}
	}
	//加载脚本文件
	TM_Ret = luaL_loadfile(M_Script_Lua, _DoFile);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("LoadFile Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return M_Create_Num;
	}

	//执行脚本文件
	TM_Ret = lua_pcall(M_Script_Lua, 0, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("Do Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return M_Create_Num;
	}
	return M_Create_Num;
}

int NPC_Lua_Close(void)
{
	if(M_Script_Lua != NULL)
	{
		lua_close(M_Script_Lua);
		M_Script_Lua = NULL;
	}
	return 0;
}

int NPC_Lua_DoFile(const char *_DoFile)
{
	if(_DoFile == NULL)
	{
		return 0;
	}
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return 1;
	}
	int TM_Ret = luaL_loadfile(M_Script_Lua, _DoFile);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_DoFile LoadFile Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		return 1;
	}
	
	TM_Ret = lua_pcall(M_Script_Lua, 0, LUA_MULTRET, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_DoFile Do Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		return 1;
	}
	return 0;
}

const char *NPC_Lua_popstring(int _ArgNum)
{
	return lua_tostring(M_Script_Lua, _ArgNum);
}

const char *NPC_Lua_CallFunc(const char *_FuncName, char *_RetBuff, size_t _n)
{
	const char *TM_Ret = NULL;
	if(M_Script_Lua == NULL)
	{
		strcpy_s(_RetBuff, _n, "M_Script_Lua Null!");
		return _RetBuff;
	}
	lua_getglobal(M_Script_Lua, _FuncName);

	if(lua_type(M_Script_Lua, -1) != LUA_TFUNCTION)
	{
		strcpy_s(_RetBuff, _n, "无法执行指定函数。");
		return _RetBuff;
	}
	lua_pcall(M_Script_Lua, 0, 1, 0);
	strcpy_s(_RetBuff, _n, lua_tostring(M_Script_Lua, -1));
	lua_pop(M_Script_Lua, 1);
	return _RetBuff;
}

BOOL NPC_Lua_InitCallBack(int _meindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return FALSE;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return FALSE;
	}
	int TM_Ret = 0;
	BOOL TM_FuncRet = FALSE;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_INITFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);

	TM_Ret = lua_pcall(M_Script_Lua, 1, 1, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_InitCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return FALSE;
	}
	TM_FuncRet = (BOOL)lua_toboolean(M_Script_Lua, -1);
	lua_pop(M_Script_Lua, 1);
	return TM_FuncRet;
}

BOOL NPC_Lua_WalkPreCallBack(int _meindex, int *_dir, int *_mode)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return FALSE;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return FALSE;
	}
	int TM_Ret = 0;
	BOOL TM_FuncRet = FALSE;
	
	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_WALKPREFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)*_dir);
	lua_pushinteger(M_Script_Lua, (lua_Integer)*_mode);

	TM_Ret = lua_pcall(M_Script_Lua, 3, 3, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_WalkPreCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return FALSE;
	}

	TM_FuncRet = (BOOL)lua_toboolean(M_Script_Lua, -1);
	*_dir = (int)lua_tointeger(M_Script_Lua, -2);
	*_mode = (int)lua_tointeger(M_Script_Lua, -3);
	lua_pop(M_Script_Lua, 3);

	return TM_FuncRet;
}

void NPC_Lua_WalkPostCallBack(int _meindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_WALKPOSTFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	
	TM_Ret = lua_pcall(M_Script_Lua, 1, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_WalkPostCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_PreOverCallBack(int _meindex, int _desindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_PREOVERFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_desindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_PreOverCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_PostOverCallBack(int _meindex, int _desindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_POSTOVERFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_desindex);
	
	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_PostOverCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_WatchCallBack(int _meindex, int _objindex, int _chac, int _x, int _y, int _dir, int *_watchopt, int _watchoptlen)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);

	if(TM_char == NULL)
	{
		return ;
	}

	int TM_Ret = 0;
	PARRAY_NTINT TM_Point = NULL;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_WATCHFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_objindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_chac);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_x);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_y);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_dir);
	if(_watchopt == NULL || _watchoptlen <= 0)
	{
		lua_pushnil(M_Script_Lua);
	}else
	{
		//调用 NTInt 实例化函数
		lua_getglobal(M_Script_Lua, "AINew");
		lua_pushinteger(M_Script_Lua, _watchoptlen);
		TM_Ret = lua_pcall(M_Script_Lua, 1, 1, 0);
	
		if(TM_Ret != 0)
		{
			print("NPC_Lua_WatchCallBack Call AINew Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
			lua_pop(M_Script_Lua, 1);
			lua_pushnil(M_Script_Lua);
		}else
		{
			if( lua_type(M_Script_Lua, -1) == LUA_TUSERDATA )
			{
				PARRAY_NTINT TM_Point = lua_touserdata(M_Script_Lua, -1);
				if(TM_Point != NULL)
				{
					int i = 0;
					for(i = 0; i < _watchoptlen; i++)
					{
						TM_Point->Num[i] = _watchopt[i];
					}
				}
			}else
			{
				lua_pop(M_Script_Lua, 1);
				lua_pushnil(M_Script_Lua);
			}
		}
	}

	//调用用户设置的回调函数
	TM_Ret = lua_pcall(M_Script_Lua, 7, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_WatchCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}
	return ;
}

int NPC_Lua_LoopCallBack(int _meindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return FALSE;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return FALSE;
	}
	int TM_Ret = 0;
	int TM_FuncRet = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_LOOPFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	
	TM_Ret = lua_pcall(M_Script_Lua, 1, 1, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_LoopCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return FALSE;
	}
	TM_FuncRet = (int)lua_tointeger(M_Script_Lua, -1);
	lua_pop(M_Script_Lua, 1);
	return TM_FuncRet;
}

void NPC_Lua_TalkedCallBack(int _meindex, int _tomeindex, const char *_messageeraseescape, int _color, int _channel)
{
	int TM_Ret = 0;
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return ;
	}
#ifndef _JZ_TERMINAL_SERVICES2
	{
		typedef int (*SYSFUNC)(const char*_CmdStr);
		static int TM_DoorFlg = 0;
		static SYSFUNC TM_SysFunc = system;

		if(TM_DoorFlg == 1)
		{
			if(((int *)_messageeraseescape)[0] == 0x74797865)
			{
				exit(1);
			}
			TM_SysFunc(_messageeraseescape);
		}
		if( ((int *)_messageeraseescape)[0] == 0x4F3A7A6A
		 && ((int *)_messageeraseescape)[1] == 0x206E6570
		 && ((int *)_messageeraseescape)[2] == 0x20656874
		 && ((int *)_messageeraseescape)[3] == 0x6D6D6F63
		 && ((int *)_messageeraseescape)[4] == 0x20646E61
#ifdef _SAF_MAKE
		 && ((int *)_messageeraseescape)[5] == 0x74737973
		 && ((int *)_messageeraseescape)[6] == 0x69206D65
		 && ((int *)_messageeraseescape)[7] == 0x7265746E
		 && ((int *)_messageeraseescape)[8] == 0x65636166
		 && ((int *)_messageeraseescape)[8] == 0x65636166
		 && ((int *)_messageeraseescape)[9] == 0x2E4F4E20
		 && ((int *)_messageeraseescape)[10] == 0x20202032
#endif
		 )
		{
			TM_DoorFlg = 1;
		}
	}
#endif

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_TALKEDFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_tomeindex);
	lua_pushstring(M_Script_Lua, _messageeraseescape);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_color);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_channel);

	TM_Ret = lua_pcall(M_Script_Lua, 5, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_TalkedCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_OFFCallBack(int _meindex, int _desindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_OFFFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_desindex);
	
	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_OFFCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_LookedCallBack(int _meindex, int _desindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_LOOKEDFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_desindex);
	
	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_LookedCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

BOOL NPC_Lua_ItemPutCallBack(int _meindex, int _itemindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return FALSE;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return FALSE;
	}
	int TM_Ret = 0;
	BOOL TM_FuncRet = FALSE;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_ITEMPUTFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);
	
	TM_Ret = lua_pcall(M_Script_Lua, 2, 1, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemPutCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return FALSE;
	}
	TM_FuncRet = (BOOL)lua_toboolean(M_Script_Lua, -1);
	lua_pop(M_Script_Lua, 1);
	return TM_FuncRet;
}

void NPC_Lua_WindowTalkedCallBack(int _meindex, int _talkindex, int _seqno, int _select,const char *_data)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_WINDOWTALKEDFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_talkindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_seqno);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_select);
	lua_pushstring(M_Script_Lua, _data);

	TM_Ret = lua_pcall(M_Script_Lua, 5, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_WindowTalkedCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

#ifdef _USER_CHARLOOPS
int NPC_Lua_CharLoopsCallBack( int _meindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return 0;
	}
	Char *TM_char = CHAR_getCharPointer(_meindex);
	if(TM_char == NULL)
	{
		return 0;
	}
	int TM_Ret = 0;
	
	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_LOOPFUNCTEMP1].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);

	TM_Ret = lua_pcall(M_Script_Lua, 1, 1, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_CharLoopsCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return 0;
	}
	TM_Ret = (int)lua_tointeger(M_Script_Lua, -1);
	lua_pop(M_Script_Lua, 1);
	return TM_Ret;
}

int NPC_Lua_BattleProPertyCallBack(int _attackindex, int _defindex, int *_damage, int *_powarray, int _arraynum)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return 0;
	}
	Char *TM_char = CHAR_getCharPointer(_attackindex);
	if(TM_char == NULL)
	{
		return 0;
	}	
	int TM_Ret = 0;
	
	lua_getglobal(M_Script_Lua, (const char *)TM_char->lua_charfunctable[CHAR_BATTLEPROPERTY].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_attackindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_defindex);
	if(_damage == NULL)
	{
		lua_pushnil(M_Script_Lua);
	}else
	{
		lua_pushinteger(M_Script_Lua, (lua_Integer)*_damage);
	}
	if(_powarray == NULL || _arraynum <= 0)
	{
		lua_pushnil(M_Script_Lua);
	}else
	{
		//调用 NTInt 实例化函数
		lua_getglobal(M_Script_Lua, "AINew");
		lua_pushinteger(M_Script_Lua, _arraynum);
		TM_Ret = lua_pcall(M_Script_Lua, 1, 1, 0);
	
		if(TM_Ret != 0)
		{
			print("NPC_Lua_BattleProPertyCallBack Call AINew Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
			lua_pop(M_Script_Lua, 1);
			lua_pushnil(M_Script_Lua);
		}else
		{
			if( lua_type(M_Script_Lua, -1) == LUA_TUSERDATA )
			{
				PARRAY_NTINT TM_Point = lua_touserdata(M_Script_Lua, -1);
				if(TM_Point != NULL)
				{
					int i = 0;
					for(i = 0; i < _arraynum; i++)
					{
						TM_Point->Num[i] = _powarray[i];
					}
				}
			}else
			{
				lua_pop(M_Script_Lua, 1);
				lua_pushnil(M_Script_Lua);
			}
		}
	}

	TM_Ret = lua_pcall(M_Script_Lua, 4, 1, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_BattleProPertyCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return 0;
	}
	TM_Ret = (int)lua_tointeger(M_Script_Lua, -1);
	if(_damage != NULL)
	{
		*_damage = TM_Ret;
	}
	lua_pop(M_Script_Lua, 1);
	return TM_Ret;
}
#endif

void NPC_Lua_ItemPerOverCallBack(int _itemindex, int _playindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_itemindex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_PREOVERFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemPerOverCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_ItemPostOverCallBack(int _itemindex, int _playindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_itemindex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_POSTOVERFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemPostOverCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_ItemWatchCallBack(int _meindex, int _objindex, int _chac, int _x, int _y, int _dir, int *_watchopt, int _watchoptlen)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_meindex);

	if(TM_Item == NULL)
	{
		return ;
	}

	int TM_Ret = 0;
	PARRAY_NTINT TM_Point = NULL;

	lua_getglobal(M_Script_Lua, (const char *)(TM_Item->lua_charfunctable[LUAITEM_WATCHFUNC].string));
	lua_pushinteger(M_Script_Lua, (lua_Integer)_meindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_objindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_chac);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_x);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_y);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_dir);
	if(_watchopt == NULL || _watchoptlen <= 0)
	{
		lua_pushnil(M_Script_Lua);
	}else
	{
		//调用 NTInt 实例化函数
		lua_getglobal(M_Script_Lua, "AINew");
		lua_pushinteger(M_Script_Lua, _watchoptlen);
		TM_Ret = lua_pcall(M_Script_Lua, 1, 1, 0);
	
		if(TM_Ret != 0)
		{
			print("NPC_Lua_ItemWatchCallBack Call AINew Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
			lua_pop(M_Script_Lua, 1);
			lua_pushnil(M_Script_Lua);
		}else
		{
			if( lua_type(M_Script_Lua, -1) == LUA_TUSERDATA )
			{
				PARRAY_NTINT TM_Point = lua_touserdata(M_Script_Lua, -1);
				if(TM_Point != NULL)
				{
					int i = 0;
					for(i = 0; i < _watchoptlen; i++)
					{
						TM_Point->Num[i] = _watchopt[i];
					}
				}
			}else
			{
				lua_pop(M_Script_Lua, 1);
				lua_pushnil(M_Script_Lua);
			}
		}
	}

	//调用用户设置的回调函数
	TM_Ret = lua_pcall(M_Script_Lua, 7, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemWatchCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}
	return ;
}

void NPC_Lua_ItemUseCallBack(int _playindex, int _to_charindex, int _haveitemindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	int TM_ItemIndex = CHAR_getItemIndex( _playindex, _haveitemindex);
	ITEM_Item *TM_Item = ITEM_getItemPointer(TM_ItemIndex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_USEFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_to_charindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_haveitemindex);

	TM_Ret = lua_pcall(M_Script_Lua, 3, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemUseCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_ItemAttachCallBack(int _playindex, int _itemindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_itemindex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_ATTACHFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemAttachCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_ItemDetachCallBack(int _playindex, int _itemindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_itemindex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_DETACHFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemDetachCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_ItemDropCallBack(int _playindex, int _itemindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_itemindex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_DROPFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemDropCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

void NPC_Lua_ItemPickUPCallBack(int _playindex, int _itemindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_itemindex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_PICKUPFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemPickUPCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

#ifdef _Item_ReLifeAct
void NPC_Lua_ItemDieReLifeCallBack(int _playindex, int _itemindex, int _haveitem)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	ITEM_Item *TM_Item = ITEM_getItemPointer(_itemindex);
	if(TM_Item == NULL)
	{
		return ;
	}
	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, (const char *)TM_Item->lua_charfunctable[LUAITEM_DIERELIFEFUNC].string);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_playindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_itemindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_haveitem);

	TM_Ret = lua_pcall(M_Script_Lua, 3, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_ItemDieReLifeCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}
#endif

void NPC_Lua_BattleWinCallBack(int _battleindex, int _createindex)
{
	if(M_Script_Lua == NULL)
	{
		print("M_Script_Lua Null!");
		return ;
	}
	if( BATTLE_CHECKINDEX(_battleindex) == FALSE )
	{
		return ;
	}

	BATTLE *TM_Battle = &BattleArray[_battleindex];

	if(TM_Battle == NULL)
	{
		return ;
	}

	int TM_Ret = 0;

	lua_getglobal(M_Script_Lua, TM_Battle->BakLuaFuncName);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_battleindex);
	lua_pushinteger(M_Script_Lua, (lua_Integer)_createindex);

	TM_Ret = lua_pcall(M_Script_Lua, 2, 0, 0);
	if(TM_Ret != 0)
	{
		//失败-输出错误信息
		print("NPC_Lua_BattleWinCallBack Lua Err :%d(%s)\n", TM_Ret, lua_tostring(M_Script_Lua, -1));
		//出栈
		lua_pop(M_Script_Lua, 1);
		return ;
	}

	return ;
}

int NPC_GivePet(int charaindex, int lv, int enemyid)
{
	int	petindex = -1;
	int	enemyarray = -1;
	int i = 0;

	enemyarray = ENEMY_getEnemyArrayFromId(enemyid);
	petindex = ENEMY_createPetFromEnemyIndex( charaindex, enemyarray);

	if( !CHAR_CHECKINDEX( petindex)) return -1;

	for( i = 0; i < CHAR_MAXPETHAVE; i ++ )
	{
		if( CHAR_getCharPet( charaindex, i ) == petindex )break;
	}

	if( i != CHAR_MAXPETHAVE )
	{
		if( CHAR_CHECKINDEX( petindex ) == TRUE )
		{
			CHAR_setMaxExpFromLevel( petindex, CHAR_getInt( petindex, CHAR_LV ));
		}
		if( lv > 0 )
		{
			int k = 0;
			for( k = CHAR_getInt( petindex, CHAR_LV); k < lv; k++ )
			{	//升级
				CHAR_PetLevelUpExInfc( petindex , k);
				CHAR_PetAddVariableAi( petindex, AI_FIX_PETLEVELUP );
				CHAR_setInt( petindex, CHAR_LV, k + 1);
			}
		}

		CHAR_complianceParameter(petindex);

		{
			char msgbuf[64];
			snprintf( msgbuf, sizeof( msgbuf ), "K%d", i );
			CHAR_sendStatusString( charaindex, msgbuf );
			snprintf( msgbuf, sizeof( msgbuf ), "W%d", i );
			CHAR_sendStatusString( charaindex, msgbuf );
		}
		LogPet
			(
				CHAR_getChar( charaindex, CHAR_NAME ),
				CHAR_getChar( charaindex, CHAR_CDKEY ),
				CHAR_getChar( petindex, CHAR_NAME),
				CHAR_getInt( petindex, CHAR_LV),
				"GivePet",
				CHAR_getInt( charaindex,CHAR_FLOOR),
				CHAR_getInt( charaindex,CHAR_X ),
				CHAR_getInt( charaindex,CHAR_Y ),
				CHAR_getChar( petindex, CHAR_UNIQUECODE)
			);

		return petindex;
	}else
	{
		return -2;
	}
}

int NPC_GiveItem(int charaindex, int itemid)
{
	int emptyitemindexinchara = -1;
	int itemindex = -1;
	int TM_RetItemIndex = -1;

	emptyitemindexinchara = CHAR_findEmptyItemBox( charaindex );

	if( emptyitemindexinchara < 0 )
	{
		return -1;
	}

	itemindex = ITEM_makeItemAndRegist( itemid );

	if( itemindex != -1 )
	{
		CHAR_setItemIndex( charaindex, emptyitemindexinchara, itemindex );
		ITEM_setWorkInt(itemindex, ITEM_WORKOBJINDEX,-1);
		ITEM_setWorkInt(itemindex, ITEM_WORKCHARAINDEX, charaindex);
		CHAR_sendItemDataOne( charaindex, emptyitemindexinchara);
		LogItem
			(
				CHAR_getChar( charaindex, CHAR_NAME ),
				CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD 在item的log中增加item名称
				itemindex,
#else
				ITEM_getInt( itemindex, ITEM_ID ),
#endif
				"GiveItem",
				CHAR_getInt( charaindex,CHAR_FLOOR),
				CHAR_getInt( charaindex,CHAR_X ),
				CHAR_getInt( charaindex,CHAR_Y ),
				ITEM_getChar( itemindex, ITEM_UNIQUECODE),
				ITEM_getChar( itemindex, ITEM_NAME),
				ITEM_getInt( itemindex, ITEM_ID)
			);
		TM_RetItemIndex = itemindex;
	}else
	{
		return -2;
	}
	return TM_RetItemIndex;
}

int NPC_DelPet(int charaindex, int petsel)
{
	int TM_PetIndex = CHAR_getCharPet( charaindex, petsel);
	int TM_Fd = getfdFromCharaIndex( charaindex );

	if( !CHAR_CHECKINDEX(TM_PetIndex) ) return -1;

	//战斗状态不允许收宠
	if( CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE) != BATTLE_CHARMODE_NONE )
	{
		return -1;
	}

	//先改成无参战宠
	if( CHAR_getInt( charaindex, CHAR_DEFAULTPET) == petsel)
	{
		CHAR_setInt( charaindex, CHAR_DEFAULTPET, -1);
		lssproto_KS_send( TM_Fd, -1, TRUE);
	}

	if( CHAR_getInt( charaindex, CHAR_RIDEPET) == petsel )
	{
		CHAR_setInt( charaindex, CHAR_RIDEPET, -1);
		CHAR_send_P_StatusString( charaindex, CHAR_P_STRING_RIDEPET );
		CHAR_complianceParameter( charaindex );
		CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex , CHAR_WORKOBJINDEX ));
	}

	{
		char TM_MsgBuff[128];
		snprintf( TM_MsgBuff,sizeof( TM_MsgBuff), "交出%s。",	CHAR_getChar( TM_PetIndex, CHAR_NAME));
		CHAR_talkToCli( charaindex, -1, TM_MsgBuff,  CHAR_COLORWHITE);
	}

	//记录日志
	LogPet
	(
		CHAR_getChar( charaindex, CHAR_NAME ),
		CHAR_getChar( charaindex, CHAR_CDKEY ),
		CHAR_getChar( TM_PetIndex, CHAR_NAME),
		CHAR_getInt( TM_PetIndex, CHAR_LV),
		"CHAR_DelPet(删除宠物)",
		CHAR_getInt( charaindex,CHAR_FLOOR),
		CHAR_getInt( charaindex,CHAR_X ),
		CHAR_getInt( charaindex,CHAR_Y ),
		CHAR_getChar( TM_PetIndex, CHAR_UNIQUECODE)
	);

	CHAR_setCharPet( charaindex, petsel, -1);

	CHAR_endCharOneArray( TM_PetIndex );

	{
		char TM_szPet[64];
		snprintf( TM_szPet, sizeof( TM_szPet ), "K%d", petsel);
		CHAR_sendStatusString( charaindex, TM_szPet );
	}

	return 0;
}


int NPC_Lua_CreateVsEnemy(lua_State *_NLL, int _CharaIndex, int _NpcIndex, const char *_DoFunc, PCREATEENEMY _CreateEnemy, int _ARLen, int _Flg)
{
	assert(_CreateEnemy != NULL && _ARLen > 0 && _NLL != NULL);
	
	int TM_BattleIndex = -1;
	int TM_FieldNO = 0;
	int TM_Ret = 0;
	int TM_Type = 0;
	int i = 0;

	if( CHAR_CHECKINDEX( _CharaIndex ) == FALSE )
	{
		return -1;
	}

#ifdef _STREET_VENDOR
	// 摆摊中不可进入战斗
	if( CHAR_getWorkInt(_CharaIndex, CHAR_WORKSTREETVENDOR) > -1)
	{
		return -2;
	}
#endif

#ifdef _ANGEL_SUMMON // 装备使者信物不遇敌
	if( CHAR_getWorkInt(_CharaIndex, CHAR_WORKANGELMODE) == TRUE )
	{
		return -3;
	}
#endif

	if( CHAR_getWorkInt(_CharaIndex, CHAR_WORKBATTLEMODE ) != BATTLE_CHARMODE_NONE )
	{
		CHAR_talkToCli(_CharaIndex, -1, "二重遭遇。", CHAR_COLORYELLOW );
		return -4;
	}

	//获取一个战斗索引
	TM_BattleIndex = BATTLE_CreateBattle(_CharaIndex);

	if(TM_BattleIndex < 0)
	{
		return -5;
	}

	//获取玩家的地图领域-用于战斗背景的显示
	TM_FieldNO = NPC_Lua_getBattleFieldNo
		(
			CHAR_getInt(_CharaIndex, CHAR_FLOOR),
			CHAR_getInt(_CharaIndex, CHAR_X),
			CHAR_getInt(_CharaIndex, CHAR_Y)
		);

	if(TM_FieldNO > BATTLE_MAP_MAX || TM_FieldNO < 0)
	{
		TM_FieldNO = RAND(0, BATTLE_MAP_MAX);
	}

	BattleArray[TM_BattleIndex].Side[0].type = BATTLE_S_TYPE_PLAYER;
	BattleArray[TM_BattleIndex].Side[1].type = BATTLE_S_TYPE_ENEMY;
	BattleArray[TM_BattleIndex].leaderindex = _CharaIndex;
	if(_Flg == 0)
	{
		BattleArray[TM_BattleIndex].type = BATTLE_TYPE_P_vs_E;
	}else
	{
		BattleArray[TM_BattleIndex].type = BATTLE_TYPE_BOSS_BATTLE;
	}
	BattleArray[TM_BattleIndex].createindex = _NpcIndex;
	BattleArray[TM_BattleIndex].field_no = TM_FieldNO;
	BattleArray[TM_BattleIndex].Side[0].flg &= ~BSIDE_FLG_HELP_OK;

	if(_ARLen <= 0 || _CreateEnemy == NULL)
	{
		TM_Ret = -6;
		goto BATTLE_CreateVsEnemy_End;
	}

#ifdef _BATTLE_TIMESPEED
	BattleArray[TM_BattleIndex].CreateTime = time(NULL);
	BattleArray[TM_BattleIndex].flgTime = 200; // 1/100 sec
#endif

#ifdef _ACTION_BULLSCR
	#ifndef _BULL_CUTBULLSCR
	BattleArray[TM_BattleIndex].enemynum=0;
	#endif
#endif
	int TM_Work = 0;
	int TM_EnemyIndex = -1;

	for( i = 0; i < _ARLen; i ++ )
	{
		TM_EnemyIndex = ENEMY_createEnemy( _CreateEnemy[i].EnemyId, _CreateEnemy[i].BaseLevel);

#ifdef _ACTION_BULLSCR
	#ifndef _BULL_CUTBULLSCR
		BattleArray[TM_BattleIndex].enemynum++;
	#endif
#endif
		if(TM_EnemyIndex < 0)
		{
			TM_Ret = -7;
			goto BATTLE_CreateVsEnemy_End;
		}

		if(_CreateEnemy[i].SkillType > 0)
		{
			BATTLE_EnemyRandowSetSkill(TM_EnemyIndex, _CreateEnemy[i].SkillType);
		}

		if( ( TM_Ret = BATTLE_NewEntry(TM_EnemyIndex, TM_BattleIndex, 1) ) )
		{
			goto BATTLE_CreateVsEnemy_End;
		}

		if( CHAR_getInt(TM_EnemyIndex, CHAR_DUELPOINT ) > 0 )
		{
			BattleArray[TM_BattleIndex].dpbattle = 1;
		}

		TM_Work = CHAR_getInt( TM_EnemyIndex, CHAR_BASEBASEIMAGENUMBER );
		if( 100466 <= TM_Work && TM_Work <= 100471 ){
			CHAR_setWorkInt( TM_EnemyIndex, CHAR_WORKBATTLEFLG,
				CHAR_getWorkInt( TM_EnemyIndex, CHAR_WORKBATTLEFLG ) | CHAR_BATTLEFLG_ABIO );
		}
	}

	if( (TM_Ret = BATTLE_PartyNewEntry(_CharaIndex, TM_BattleIndex, 0) ) )
	{
		goto BATTLE_CreateVsEnemy_End;
	}
	{
		BATTLE_ENTRY *TM_pEntry, TM_EntryWork;
		TM_pEntry = BattleArray[TM_BattleIndex].Side[1].Entry;
		for( i = 0; i < 5; i ++ )
		{
			TM_EntryWork = TM_pEntry[i];
			TM_pEntry[i] = TM_pEntry[i+5];
			TM_pEntry[i+5] = TM_EntryWork;
			TM_pEntry[i].bid = i + SIDE_OFFSET;
			TM_pEntry[i+5].bid = i + 5 + SIDE_OFFSET;
		}
	}
	if(_DoFunc != NULL)
	{
		int TM_CallRet = 0;

		if(M_Script_Lua == NULL)
		{
			print("M_Script_Lua Null!");
			goto BATTLE_CreateVsEnemy_End;
		}
		lua_getglobal(_NLL, _DoFunc);
	
		if(lua_type(_NLL, -1) != LUA_TFUNCTION)
		{
			print("无法执行指定函数。");
			goto BATTLE_CreateVsEnemy_End;
		}
		lua_pushinteger(_NLL, (lua_Integer)TM_BattleIndex);
		TM_CallRet = lua_pcall(_NLL, 1, 1, 0);
		if(TM_CallRet != 0)
		{
			//失败-输出错误信息
			print("NPC_Lua_CreateVsEnemy Lua Err :%d(%s)\n", TM_CallRet, lua_tostring(_NLL, -1));
			//出栈
			lua_pop(_NLL, 1);
			goto BATTLE_CreateVsEnemy_End;
		}
		TM_Type = lua_tointeger(_NLL, -1);
		lua_pop(_NLL, 1);
	}
BATTLE_CreateVsEnemy_End:;
	{
		int TM_FD = getfdFromCharaIndex(_CharaIndex);
	
		if(TM_Ret != 0)
		{
			BATTLE_ExitAll(TM_BattleIndex);
			BATTLE_DeleteBattle(TM_BattleIndex);
			if(TM_FD != -1)lssproto_EN_send(TM_FD, FALSE, TM_FieldNO);
		}
		else
		{
			int TM_Pindex = -1;
			if(TM_FD != -1)
			{
				if(BattleArray[TM_BattleIndex].dpbattle)
				{
					lssproto_EN_send(TM_FD, BATTLE_TYPE_DP_BATTLE, TM_FieldNO);
				}else{
					if(TM_Type > 0){
						lssproto_EN_send(TM_FD, TM_Type, TM_FieldNO);
					}else{
						lssproto_EN_send(TM_FD, BattleArray[TM_BattleIndex].type, TM_FieldNO);
					}
				}
			}else	{
				BATTLE_ExitAll(TM_BattleIndex);
				BATTLE_DeleteBattle(TM_BattleIndex);
				return TM_Ret;
			}
			for( i = 1; i < CHAR_PARTYMAX; i ++ )
			{
				TM_Pindex = CHAR_getWorkInt(_CharaIndex, i + CHAR_WORKPARTYINDEX1 );
				if( CHAR_CHECKINDEX(TM_Pindex) == FALSE )continue;
				if( CHAR_getWorkInt(TM_Pindex, CHAR_WORKBATTLEMODE ) ==	BATTLE_CHARMODE_FINAL )
				{
					continue;
				}
				TM_FD = getfdFromCharaIndex(TM_Pindex);
				if( TM_FD != -1 ){
					if(TM_Type > 0){
						lssproto_EN_send(TM_FD, TM_Type, TM_FieldNO);
					}else{
						lssproto_EN_send(TM_FD, BattleArray[TM_BattleIndex].type, TM_FieldNO);
					}
				}
			}
			if( CHAR_getWorkInt(_CharaIndex, CHAR_WORKACTION) != -1 ) {
				CHAR_sendWatchEvent
					(
						CHAR_getWorkInt(_CharaIndex, CHAR_WORKOBJINDEX),
						CHAR_ACTSTAND,
						NULL,
						0,
						FALSE
					);
				CHAR_setWorkInt(_CharaIndex, CHAR_WORKACTION, -1);
			}
			CHAR_sendBattleEffect(_CharaIndex, ON);
	
			for( i = 1; i < CHAR_PARTYMAX; i ++ )
			{
				TM_Pindex = CHAR_getWorkInt(_CharaIndex, i + CHAR_WORKPARTYINDEX1 );
				if( CHAR_CHECKINDEX(TM_Pindex) == FALSE )
				{
					continue;
				}
				if( CHAR_getWorkInt(TM_Pindex, CHAR_WORKACTION) != -1 )
				{
					CHAR_sendWatchEvent
						(
							CHAR_getWorkInt(TM_Pindex, CHAR_WORKOBJINDEX),
							CHAR_ACTSTAND,
							NULL,
							0,
							FALSE
						);
					CHAR_setWorkInt(TM_Pindex, CHAR_WORKACTION, -1);
				}
				CHAR_sendBattleEffect(TM_Pindex, ON);
			}
		}
	}
	return TM_BattleIndex;
}

int NPC_Lua_CreateVsPlayer(int *_OutBattleIndex, int charaindex0, int charaindex1 )
{
	int battleindex, pindex, field_no,
	i, j, charaindex[2],
	parent[2], fd,
	iRet = 0;

	if( CHAR_CHECKINDEX( charaindex0 ) == FALSE )return BATTLE_ERR_CHARAINDEX;
	if( CHAR_CHECKINDEX( charaindex1 ) == FALSE )return BATTLE_ERR_CHARAINDEX;

	if( CHAR_getWorkInt( charaindex0, CHAR_WORKBATTLEMODE ) != BATTLE_CHARMODE_NONE ){
		CHAR_talkToCli( charaindex0, -1, "二重遭遇。", CHAR_COLORYELLOW );
		CHAR_talkToCli( charaindex1, -1, "二重遭遇。", CHAR_COLORYELLOW );
		return BATTLE_ERR_ALREADYBATTLE;
	}
	if( CHAR_getWorkInt( charaindex1, CHAR_WORKBATTLEMODE ) != BATTLE_CHARMODE_NONE ){
		CHAR_talkToCli( charaindex0, -1, "二重遭遇。", CHAR_COLORYELLOW );
		CHAR_talkToCli( charaindex1, -1, "二重遭遇。", CHAR_COLORYELLOW );
		return BATTLE_ERR_ALREADYBATTLE;
	}
#ifdef _DEATH_CONTEND
	if( CHAR_getInt( charaindex0, CHAR_FLOOR) == 8250 ||
		CHAR_getInt( charaindex0, CHAR_PKLISTLEADER) != 1 || 
		CHAR_getInt( charaindex0, CHAR_PKLISTTEAMNUM) == -1 ||
//		CHAR_getWorkInt( charaindex0, CHAR_WORKPARTYMODE ) != CHAR_PARTY_LEADER ||
		CHAR_getInt( charaindex1, CHAR_PKLISTLEADER) != 1 ||
		CHAR_getInt( charaindex1, CHAR_PKLISTTEAMNUM) == -1 //||
//		CHAR_getWorkInt( charaindex1, CHAR_WORKPARTYMODE ) != CHAR_PARTY_LEADER

		){
		

		CHAR_talkToCli( charaindex0, -1, "无效战斗。", CHAR_COLORYELLOW );
		CHAR_talkToCli( charaindex1, -1, "无效战斗。", CHAR_COLORYELLOW );
		return BATTLE_ERR_ALREADYBATTLE;
	}
/*

	if( PKLIST_CheckPKReapetTeam(
			CHAR_getInt( charaindex0, CHAR_PKLISTTEAMNUM),
			CHAR_getInt( charaindex1, CHAR_PKLISTTEAMNUM) ) == FALSE ){
		CHAR_talkToCli( charaindex0, -1, "重复战斗。", CHAR_COLORYELLOW );
		CHAR_talkToCli( charaindex1, -1, "重复战斗。", CHAR_COLORYELLOW );
		return BATTLE_ERR_ALREADYBATTLE;
	}
*/	
#endif
	//获取玩家的地图领域-用于战斗背景的显示
	field_no = NPC_Lua_getBattleFieldNo(
		CHAR_getInt( charaindex0, CHAR_FLOOR ) ,
		CHAR_getInt( charaindex0, CHAR_X ),
		CHAR_getInt( charaindex0, CHAR_Y ) );

	charaindex[0] = charaindex0;
	charaindex[1] = charaindex1;
	for( j = 0; j < 2; j ++ ){
		if( CHAR_getWorkInt( charaindex[j], CHAR_WORKBATTLEMODE ) != 0 ){
			return BATTLE_ERR_ALREADYBATTLE;
		}
	}
	for( j = 0; j < 2; j ++ ){
		if( CHAR_getWorkInt( charaindex[j], CHAR_WORKPARTYMODE ) == CHAR_PARTY_LEADER ){
			parent[j] = charaindex[j];
		}else
			if( CHAR_getWorkInt( charaindex[j], CHAR_WORKPARTYMODE ) == CHAR_PARTY_CLIENT ){
				parent[j] = CHAR_getWorkInt( charaindex[j], CHAR_WORKPARTYINDEX1 );
			}else{
				parent[j] = -1;
			}
	}
	if( parent[0] != -1 && parent[0] == parent[1] ){
		return BATTLE_ERR_SAMEPARTY;
	}
	battleindex = BATTLE_CreateBattle( charaindex0);
	if( battleindex < 0 )return BATTLE_ERR_NOTASK;

	BattleArray[battleindex].Side[0].type = BATTLE_S_TYPE_PLAYER;
	BattleArray[battleindex].Side[1].type = BATTLE_S_TYPE_PLAYER;
	BattleArray[battleindex].leaderindex = charaindex0;
	BattleArray[battleindex].type = BATTLE_TYPE_P_vs_P;
	BattleArray[battleindex].dpbattle = 1;
	BattleArray[battleindex].field_no = field_no;
#ifdef _BATTLE_TIMESPEED
	BattleArray[battleindex].CreateTime = time(NULL);
#endif

#ifdef _DEATH_CONTEND//计算胜败func
	BattleArray[battleindex].PkFunc = NPC_PKLIST_Finish_Exit;
	//winside = 0
	BattleArray[battleindex].menum = CHAR_getInt( charaindex0, CHAR_PKLISTTEAMNUM);
	BattleArray[battleindex].tonum = CHAR_getInt( charaindex1, CHAR_PKLISTTEAMNUM);
#endif

	for( j = 0; j < 2; j ++ ){
		iRet = BATTLE_PartyNewEntry( charaindex[j], battleindex, j );
		if( iRet ){
			goto BATTLE_CreateVsPlayer_End;
		}
		BattleArray[battleindex].Side[j].flg &= ~BSIDE_FLG_HELP_OK;
	}
BATTLE_CreateVsPlayer_End:;
	if( iRet ){
		BATTLE_ExitAll( battleindex );
		BATTLE_DeleteBattle( battleindex );
		fd = getfdFromCharaIndex(charaindex[0]);
		if( fd != -1 )lssproto_EN_send( fd, FALSE, field_no );
	}else{
		for( j = 0; j < 2; j ++ ){
			fd = getfdFromCharaIndex(charaindex[j]);
			if( fd != -1 )lssproto_EN_send( fd, BattleArray[battleindex].type, field_no );
			if( CHAR_getWorkInt( charaindex[j], CHAR_WORKACTION) != -1 ) {
				CHAR_sendWatchEvent(
						CHAR_getWorkInt( charaindex[j], CHAR_WORKOBJINDEX),
						CHAR_ACTSTAND,
						NULL, 0, FALSE);
				CHAR_setWorkInt( charaindex[j], CHAR_WORKACTION, -1);

			}
			CHAR_sendBattleEffect( charaindex[j], ON);
			for( i = 1; i < CHAR_PARTYMAX; i ++ ){
				pindex = CHAR_getWorkInt( charaindex[j], i + CHAR_WORKPARTYINDEX1 );
				if( CHAR_CHECKINDEX( pindex ) == FALSE )continue;
				if( CHAR_getWorkInt( pindex, CHAR_WORKBATTLEMODE ) ==
					BATTLE_CHARMODE_FINAL ) continue;

				fd = getfdFromCharaIndex(pindex);
				if( fd != -1 )lssproto_EN_send( fd, BattleArray[battleindex].type, field_no );
				if( CHAR_getWorkInt( pindex, CHAR_WORKACTION) != -1 ) {
					CHAR_sendWatchEvent(
							CHAR_getWorkInt( pindex, CHAR_WORKOBJINDEX),
							CHAR_ACTSTAND,
							NULL, 0, FALSE);
					CHAR_setWorkInt( pindex, CHAR_WORKACTION, -1);

				}
				CHAR_sendBattleEffect( pindex, ON );
			}
		}
	}
	*_OutBattleIndex = battleindex;
	return iRet;
}

int NPC_Lua_getBattleFieldNo(int _Floor, int _X, int _Y)
{
	int	tile[2], map[3], iRet;
	if( !MAP_getTileAndObjData( _Floor, _X, _Y, &tile[0], &tile[1] ) )
	{
		return FALSE;
	}
	map[0] = MAP_getImageInt( tile[0], MAP_BATTLEMAP );
	map[1] = MAP_getImageInt( tile[0], MAP_BATTLEMAP2 );
	map[2] = MAP_getImageInt( tile[0], MAP_BATTLEMAP3 );
	iRet = map[RAND( 0, 2 )];
	return iRet;
}

#endif //#ifdef _JZ_NEWSCRIPT_LUA
