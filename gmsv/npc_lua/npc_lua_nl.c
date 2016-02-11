#include "npc_lua.h"
#include "util.h"
#include "char.h"
#include "char_base.h"
#include "anim_tbl.h"
#include "object.h"
#include "net.h"
#include "npcutil.h"
#include "npc_eventaction.h"
#include "battle.h"
#include "readmap.h"

#ifdef _JZ_NEWSCRIPT_LUA

extern char M_OutErrMsg[1024];
extern lua_State *M_Script_Lua;
extern unsigned int M_Create_Num;

//////////////////////////////////////////////////////////////////////////////
int NPC_Lua_NL_GetErrorStr(lua_State *_NLL)
{
	CheckEx(_NLL, 0);

	LRetMsg(_NLL, M_OutErrMsg);
}

int NPC_Lua_NL_CreateNpc(lua_State *_NLL)
{
	//参数有4个
	CheckEx2(_NLL, 2, 4);
	char *TM_DoFile = lua_tostring(_NLL, 1);
	char *TM_InitFuncName = lua_tostring(_NLL, 2);
	BOOL TM_IsFly = FALSE;
	char *TM_seek = NULL;
	int TM_Top = lua_gettop(_NLL);

	if(TM_Top >= 3)
	{
		TM_IsFly = (BOOL)lua_toboolean(_NLL, 3);
	}
	if(TM_Top == 4)
	{
		TM_seek = lua_tostring(_NLL, 4);
	}

	int TM_Ret = NPC_Lua_Create(TM_DoFile, TM_InitFuncName, TM_seek, TM_IsFly);

	LRet(TM_Ret);
}


int NPC_Lua_NL_DelNpc(lua_State *_NLL)
{
	CheckEx(_NLL, 1);
	CheckIndexNull(_NLL, 1);
	int TM_index = (int)lua_tointeger(_NLL, 1);
	int TM_Ret = NPC_Lua_Del(TM_index);
	LRetInt(_NLL, TM_Ret);
}
#endif //#ifdef _JZ_NEWSCRIPT_LUA
