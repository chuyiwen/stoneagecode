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

int NPC_Lua_Battle_GetPlayIndex(lua_State *_NLL)
{
	CheckEx(_NLL, 3);
	assert(BattleArray != NULL);
	CheckBattleIndexNull(_NLL, 1);
	int TM_battleindex = (int)lua_tointeger(_NLL, 1);
	int TM_side = (int)lua_tointeger(_NLL, 2);
	int TM_num = (int)lua_tointeger(_NLL, 3);
	
	if( BATTLE_CHECKINDEX( TM_battleindex ) == FALSE )
	{
		LRetErrInt(_NLL , -1, "传入的索引是无效的。");
	}

	BATTLE *TM_Battle = &BattleArray[TM_battleindex];

	if(TM_Battle == NULL)
	{
		LRetErrInt(_NLL , -2, "无法获取指针。");
	}
	int TM_Ret = -1;

	if(TM_side >= 0 && TM_side < 2 && TM_num >=0 && TM_num < BATTLE_ENTRY_MAX)
	{
		TM_Ret = TM_Battle->Side[TM_side].Entry[TM_num].charaindex;
	}else
	{
		LRetErrInt(_NLL , -3, "范围错误。");
	}
	LRetInt(_NLL, TM_Ret);
}

int NPC_Lua_Battle_SetNORisk(lua_State *_NLL)
{
	CheckEx(_NLL, 2);
	assert(BattleArray != NULL);
	CheckBattleIndexNull(_NLL, 1);
	int TM_battleindex = (int)lua_tointeger(_NLL, 1);
	int TM_vl = (int)lua_tointeger(_NLL, 2);
	
	if( BATTLE_CHECKINDEX( TM_battleindex ) == FALSE )
	{
		LRetErrInt(_NLL , -1, "传入的索引是无效的。");
	}

	BATTLE *TM_Battle = &BattleArray[TM_battleindex];

	if(TM_Battle == NULL)
	{
		LRetErrInt(_NLL , -2, "无法获取指针。");
	}
	
	int TM_Ret = TM_Battle->norisk;
	TM_Battle->norisk = TM_vl;

	LRetInt(_NLL, TM_Ret);
}

int NPC_Lua_Battle_SetMod(lua_State *_NLL)
{
	CheckEx(_NLL, 2);
	assert(BattleArray != NULL);
	CheckBattleIndexNull(_NLL, 1);
	int TM_battleindex = (int)lua_tointeger(_NLL, 1);
	int TM_vl = (int)lua_tointeger(_NLL, 2);
	
	if( BATTLE_CHECKINDEX( TM_battleindex ) == FALSE )
	{
		LRetErrInt(_NLL , -1, "传入的索引是无效的。");
	}

	BATTLE *TM_Battle = &BattleArray[TM_battleindex];

	if(TM_Battle == NULL)
	{
		LRetErrInt(_NLL , -2, "无法获取指针。");
	}
	
	int TM_Ret = TM_Battle->mode;
	TM_Battle->mode = TM_vl;

	LRetInt(_NLL, TM_Ret);
}

int NPC_Lua_Battle_SetType(lua_State *_NLL)
{
	CheckEx(_NLL, 2);
	assert(BattleArray != NULL);
	CheckBattleIndexNull(_NLL, 1);
	int TM_battleindex = (int)lua_tointeger(_NLL, 1);
	int TM_vl = (int)lua_tointeger(_NLL, 2);
	
	if( BATTLE_CHECKINDEX( TM_battleindex ) == FALSE )
	{
		LRetErrInt(_NLL , -1, "传入的索引是无效的。");
	}

	BATTLE *TM_Battle = &BattleArray[TM_battleindex];

	if(TM_Battle == NULL)
	{
		LRetErrInt(_NLL , -2, "无法获取指针。");
	}
	
	int TM_Ret = TM_Battle->type;
	TM_Battle->type = TM_vl;

	LRetInt(_NLL, TM_Ret);
}

int NPC_Lua_Battle_SetWinEvent(lua_State *_NLL)
{
	CheckEx(_NLL, 3);
	assert(BattleArray != NULL);
	CheckBattleIndexNull(_NLL, 3);
	int TM_battleindex = (int)lua_tointeger(_NLL, 3);

	if( BATTLE_CHECKINDEX( TM_battleindex ) == FALSE )
	{
		LRetErrInt(_NLL , -1, "传入的索引是无效的。");
	}

	BATTLE *TM_Battle = &BattleArray[TM_battleindex];

	if(TM_Battle == NULL)
	{
		LRetErrInt(_NLL , -2, "无法获取指针。");
	}

	int TM_Ret = NPC_Lua_GetFuncPoint(_NLL);

	if(lua_tointeger(_NLL, -1) > 0)
	{
		if( (void *)(TM_Battle->WinFunc) != (void *)NPC_Lua_BattleWinCallBack)
		{
			TM_Battle->BakFunc = TM_Battle->WinFunc;
			TM_Battle->WinFunc = (void *)NPC_Lua_BattleWinCallBack;
			strcpy_s(TM_Battle->BakLuaFuncName, sizeof(TM_Battle->BakLuaFuncName), lua_tostring(_NLL, 2));
		}
	}
	else
	{
		if((void *)(TM_Battle->WinFunc) == (void *)NPC_Lua_BattleWinCallBack)
		{
			TM_Battle->BakLuaFuncName[0] = '\0';
			TM_Battle->WinFunc = TM_Battle->BakFunc;
		}
	}
	return TM_Ret;
}

#endif //#ifdef _JZ_NEWSCRIPT_LUA
