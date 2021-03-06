/*
Omnispeak: A Commander Keen Reimplementation
Copyright (C) 2012 David Gow <david@ingeniumdigital.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "id_str.h"
#include "id_us.h"

#include "ck_act.h"
#include "ck_cross.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* The Action Manager
 * This subsystem loads the 'ACTION.CKx' file, which is a text file
 * with 'action' definitions. These are compiled into structures,
 * which have the functions loaded from the function-table.
 *
 */

#define CK_FUNCTABL_SIZE 256

STR_Table *ck_functionTable;

void CK_ACT_SetupFunctions()
{
	STR_AllocTable(&ck_functionTable, CK_FUNCTABL_SIZE);
}

void CK_ACT_AddFunction(const char *fnName, CK_ACT_Function fn)
{
	if (!STR_AddEntry(ck_functionTable, fnName, (void*)fn))
	{
		CK_Cross_LogMessage(CK_LOG_MSG_NORMAL, "Function array is full: %s is %d\n", fnName, CK_FUNCTABL_SIZE);
		Quit("AddFunction: Function table is full!");
	}
}

void CK_ACT_AddColFunction(const char *fnName, CK_ACT_ColFunction fn)
{
	if (!STR_AddEntry(ck_functionTable, fnName, (void*)fn))
	{
		CK_Cross_LogMessage(CK_LOG_MSG_NORMAL, "Function array is full: %s is %d\n", fnName, CK_FUNCTABL_SIZE);
		Quit("AddColFunction: Function table is full!");
	}
}

CK_ACT_Function CK_ACT_GetFunction(const char *fnName)
{
	if (!strcmp(fnName,"NULL")) return 0;
	CK_ACT_Function fnPtr = (CK_ACT_Function)(STR_LookupEntry(ck_functionTable, fnName));
	if (fnPtr == 0)
	{
		CK_Cross_LogMessage(CK_LOG_MSG_NORMAL, "GetFunction: Could not find function \"%s\"\n",fnName);
		Quit("GetFunction: Function not found. Check your 'ACTION.CKx' file!");
	}
	return fnPtr;
}

CK_ACT_ColFunction CK_ACT_GetColFunction(const char *fnName)
{
	if (!strcmp(fnName,"NULL")) return 0;
	CK_ACT_ColFunction fnPtr = (CK_ACT_ColFunction)(STR_LookupEntry(ck_functionTable, fnName));
	if (fnPtr == 0)
	{
		CK_Cross_LogMessage(CK_LOG_MSG_NORMAL, "GetColFunction: Could not find function \"%s\"\n",fnName);
		Quit("GetColFunction: Collision function not found. Check your 'ACTION.CKx' file!");
	}
	return fnPtr;
}

#define CK_ACT_MAXACTIONS 512

STR_Table *ck_actionTable;
CK_action *ck_actionData;
ID_MM_Arena *ck_actionStringArena;
ID_MM_Arena *ck_actionLoadTempArena;
int ck_actionsUsed;

typedef struct CK_ACT_ParserState
{
	char *data;
	int dataindex;
	int datasize;
	int linecount;
} CK_ACT_ParserState;

void CK_ACT_SetupActionDB()
{
	MM_GetPtr((mm_ptr_t*)&ck_actionData, CK_ACT_MAXACTIONS*sizeof(CK_action));
	STR_AllocTable(&ck_actionTable, CK_ACT_MAXACTIONS);
	ck_actionStringArena = MM_ArenaCreate(16384);
}

CK_action *CK_GetActionByName(const char *name)
{
	return (CK_action *)STR_LookupEntry(ck_actionTable, name);
}

CK_action *CK_GetOrCreateActionByName(const char *name)
{
	CK_action *ptr = (CK_action*)STR_LookupEntry(ck_actionTable, name);
	if (!ptr)
	{
		ptr = &ck_actionData[ck_actionsUsed++];
		char *dupName = MM_ArenaStrDup(ck_actionStringArena, name);
		STR_AddEntry(ck_actionTable, dupName, (void*)(ptr));
	}
	return ptr;
}

static char CK_ACT_PeekCharacter(CK_ACT_ParserState *ps)
{
	if (ps->dataindex >= ps->datasize) return '\0';
	return ps->data[ps->dataindex];
}

static char CK_ACT_GetCharacter(CK_ACT_ParserState *ps)
{
	if (ps->dataindex >= ps->datasize) return '\0';
	char c = ps->data[ps->dataindex++];
	if (c == '\n') ps->linecount++;
	return c;
}

static void CK_ACT_SkipWhitespace(CK_ACT_ParserState *ps)
{
	char c;
	do
	{
		c = CK_ACT_PeekCharacter(ps);
		if (c == '#')
		{
			while (CK_ACT_PeekCharacter(ps) != '\n')
			{
				c = CK_ACT_GetCharacter(ps);
			}
			c = '\n';
		}
		else if (isspace(c))
			CK_ACT_GetCharacter(ps);
	} while (c && isspace(c));
}

#define CK_ACT_MAX_TOKEN_LENGTH 64

const char *CK_ACT_GetToken(CK_ACT_ParserState *ps)
{
	char tokenbuf[CK_ACT_MAX_TOKEN_LENGTH];
	int i = 0;
	CK_ACT_SkipWhitespace(ps);
	while (CK_ACT_PeekCharacter(ps) && !isspace(CK_ACT_PeekCharacter(ps))) tokenbuf[i++] = CK_ACT_GetCharacter(ps);
	tokenbuf[i] = '\0';
	return MM_ArenaStrDup(ck_actionLoadTempArena, tokenbuf);
}



int CK_ACT_GetInteger(CK_ACT_ParserState *ps)
{
	const char *token = CK_ACT_GetToken(ps);
	int result = 0;

	/* strtol does not support the '$' prefix for hex */
	if (token[0] == '$')
		result = strtol(token+1, 0 , 16);
	else
		result = strtol(token, 0, 0);
	//printf("GetInteger %s -> %d\n", token, result);
	return result;
}

bool CK_ACT_ExpectToken(CK_ACT_ParserState *ps, const char *str)
{
	const char *c = CK_ACT_GetToken(ps);
	bool result = !strcmp(c,str);
	if (!result) CK_Cross_LogMessage(CK_LOG_MSG_WARNING, "ExpectToken, got \"%s\" expected \"%s\" on line %d\n", c, str, ps->linecount);
	return result;
}

CK_ActionType CK_ACT_GetActionType(CK_ACT_ParserState *ps)
{
	const char *tok = CK_ACT_GetToken(ps);
	CK_ActionType at = AT_UnscaledOnce;

	if (!strcmp(tok, "UnscaledOnce"))
		at = AT_UnscaledOnce;
	else if (!strcmp(tok, "ScaledOnce"))
		at = AT_ScaledOnce;
	else if (!strcmp(tok, "Frame"))
		at = AT_Frame;
	else if (!strcmp(tok, "UnscaledFrame"))
		at = AT_UnscaledFrame;
	else if (!strcmp(tok, "ScaledFrame"))
		at = AT_ScaledFrame;
	else
	{
		CK_Cross_LogMessage(CK_LOG_MSG_WARNING, "Got a bad action type %s on line %d.\n", tok, ps->linecount);
		at = (CK_ActionType)(atoi(tok));
	}
	return at;
		
}

bool CK_ACT_ParseAction(CK_ACT_ParserState *ps)
{
	if (!CK_ACT_ExpectToken(ps, "%action"))
		return false;

	const char *actName = CK_ACT_GetToken(ps);

	CK_action *act = CK_GetOrCreateActionByName(actName);

	act->compatDosPointer = CK_ACT_GetInteger(ps);
	
	act->chunkLeft = CK_ACT_GetInteger(ps);

	act->chunkRight = CK_ACT_GetInteger(ps);

	act->type = CK_ACT_GetActionType(ps);

	act->protectAnimation = CK_ACT_GetInteger(ps);

	act->stickToGround = CK_ACT_GetInteger(ps);

	act->timer = CK_ACT_GetInteger(ps);

	act->velX = CK_ACT_GetInteger(ps);
	act->velY = CK_ACT_GetInteger(ps);

	const char *cThink, *cCollide, *cDraw;
	cThink = CK_ACT_GetToken(ps);
	cCollide = CK_ACT_GetToken(ps);
	cDraw = CK_ACT_GetToken(ps);

	act->think = CK_ACT_GetFunction(cThink);
	act->collide = CK_ACT_GetColFunction(cCollide);
	act->draw = CK_ACT_GetFunction(cDraw);

	const char *nextActionName = CK_ACT_GetToken(ps);

	act->next = strcmp(nextActionName,"NULL")?CK_GetOrCreateActionByName(nextActionName):0;

	MM_ArenaReset(ck_actionLoadTempArena);
	
	return true;
}

// POTENTIALLY SLOW function - Use in game loading only!
CK_action *CK_LookupActionFrom16BitOffset(uint16_t offset)
{
	for (int i = 0; i < ck_actionsUsed; ++i)
		if (ck_actionData[i].compatDosPointer == offset)
			return &ck_actionData[i];

	return NULL;
}


void CK_ACT_LoadActions(const char *filename)
{
	ck_actionsUsed = 0;
	CK_ACT_SetupActionDB();
	int numActionsParsed = 0;
	//TODO: Parse ACTION.CKx
	CK_ACT_ParserState parserstate;
	
	CA_LoadFile(filename, (mm_ptr_t*)(&parserstate.data), &(parserstate.datasize));
	parserstate.dataindex = 0;
	parserstate.linecount = 0;

	ck_actionLoadTempArena = MM_ArenaCreate(1024);
	
	while (CK_ACT_ParseAction(&parserstate)) numActionsParsed++;

	CK_Cross_LogMessage(CK_LOG_MSG_NORMAL, "Parsed %d actions over %d lines.\n", numActionsParsed, parserstate.linecount);
	
	MM_ArenaDestroy(ck_actionLoadTempArena);
}



