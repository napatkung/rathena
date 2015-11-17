// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

/** Creates new guild and makes the 'char_id' or the invoker as guild master (player must be online)
* guild_create "<name>"{,<char_id>};
* @return 1 if attempt is success, 0 otherwise
* @author [Cydh]
*/
BUILDIN_FUNC(guild_create) {
	TBL_PC *sd = NULL;
	char guild[NAME_LENGTH];
	struct guild *g = NULL;
	int prev;

	if (script_hasdata(st,3)) {
		if (!(sd = map_charid2sd(script_getnum(st,3)))) {
			ShowError("buildin_guild_create: Player not found (CID: %d).\n", script_getnum(st,3));
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		if (!(sd = script_rid2sd(st))) {
			script_pushint(st, 0);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	safestrncpy(guild, script_getstr(st,2), NAME_LENGTH);
	if ((g = guild_searchname(guild))) {
		ShowError("buildin_guild_create: Guild is already exists (%s).\n", guild);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	prev = battle_config.guild_emperium_check;
	battle_config.guild_emperium_check = 0;
	script_pushint(st, guild_create(sd, guild));
	battle_config.guild_emperium_check = prev;
	return SCRIPT_CMD_SUCCESS;
}

/** Breaks a spesific guild by 'guild_id'
* guild_break {<guild_id>};
* @return 1 if attempt is success, 0 otherwise
* @author [Cydh]
*/
BUILDIN_FUNC(guild_break) {
	struct guild *g = NULL;
	TBL_PC *sd = NULL;

	if (script_hasdata(st,2)) {
		if (!(g = guild_search(script_getnum(st,2)))) {
			ShowError("buildin_guild_break: Guild is not exists (Guild ID: %d).\n", script_getnum(st,2));
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
		sd = map_nick2sd(g->master);
	}
	else {
		if (!(sd = script_rid2sd(st))) {
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
		if (!(g = sd->guild)) {
			ShowError("buildin_guild_break: Player isn't join in any guild (CID: %d).\n", sd->status.char_id);
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (sd)
		script_pushint(st, guild_break(sd, g->name));
	else {
		intif_guild_break(g->guild_id);
		script_pushint(st, 1);
	}
	return SCRIPT_CMD_SUCCESS;
}

/** Requests guild information
* guild_info <guild_id>,<type>;
* guild_info "<guild_id>",<type>;
* @author [Cydh]
*/
BUILDIN_FUNC(guild_info) {
	struct guild *g = NULL;
	struct script_data *data = script_getdata(st, 2);
	uint8 type = script_getnum(st, 3);
	bool val = false;

	get_val(st, data);
	if (data_isint(data)) {
		int guild_id = conv_num(st, data);
		if (!(g = guild_search(guild_id))) {
			ShowError("buildin_guild_info: Guild not found (Guild ID: %d).\n", guild_id);
			script_pushconststr(st, "");
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		char name[NAME_LENGTH];
		strcpy(name, conv_str(st,data));
		if (!(g = guild_searchname(name))) {
			ShowError("buildin_guild_info: Guild not found (Guild Name: %s).\n", name);
			script_pushint(st, -1);
			return SCRIPT_CMD_FAILURE;
		}
		val = true;
	}

	switch (type) {
		case 0: // If use guild_id, return "guild name". If use "guild name", return guild_id
			if (val)
				script_pushint(st, g->guild_id);
			else
				script_pushstrcopy(st, g->name);
			break;
		case 1: // Guild Level
			script_pushint(st, g->guild_lv);
			break;
		case 2: // Connected member
			script_pushint(st, g->connect_member);
			break;
		case 3: // Max members
			script_pushint(st, g->max_member);
			break;
		case 4: // Average member level
			script_pushint(st, g->average_lv);
			break;
		case 5: // Allies
		case 6: // Enemies
			{
				uint8 i = 0, count = 0;
				val = (type == 5); // True for ally; False for enemy
				for (i = 0; i < ARRAYLENGTH(g->alliance); i++) {
					if (!g->alliance[i].guild_id)
						continue;
					if (val && !g->alliance[i].opposition) {
						mapreg_setreg(reference_uid(add_str("$@guildallies_id"), count), g->alliance[i].guild_id);
						mapreg_setregstr(reference_uid(add_str("$@guildallies_name$"), count), g->alliance[i].name);
						count++;
					}
					else if (!val && g->alliance[i].opposition) {
						mapreg_setreg(reference_uid(add_str("$@guildenemies_id"), count), g->alliance[i].guild_id);
						mapreg_setregstr(reference_uid(add_str("$@guildenemies_name$"), count), g->alliance[i].name);
						count++;
					}
				}
				script_pushint(st, count);
			}
			break;
		case 7: // Castles
			{
				uint8 count = 0;
				struct guild_castle* gc = NULL;
				DBIterator *iter = db_iterator(guild_get_castle_db());
				for (gc = (struct guild_castle*)dbi_first(iter); dbi_exists(iter); gc = (struct guild_castle*)dbi_next(iter)) {
					if (gc->guild_id == g->guild_id) {
						mapreg_setreg(reference_uid(add_str("$@guildcastles_id"), count), gc->castle_id);
						mapreg_setregstr(reference_uid(add_str("$@guildcastles_name$"), count), gc->castle_name);
						count++;
					}
				}
				dbi_destroy(iter);
				script_pushint(st, count);
			}
			break;
		default:
			ShowDebug("buildin_guild_info: Invalid info type %d.\n", type);
			if (val)
				script_pushint(st, -1);
			else
				script_pushconststr(st, "");
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/** Adds a player to a guild (player must be online)
* guild_addmember <guild_id>{,<char_id>};
* @return 1 if attempt is success, 0 otherwise
* @author [Cydh]
*/
BUILDIN_FUNC(guild_addmember) {
	TBL_PC *sd = NULL;
	int guild_id = script_getnum(st,2);
	struct guild *g = guild_search(guild_id);

	if (!g) {
		ShowError("buildin_guild_addmember: Guild not found (ID: %d).\n", guild_id);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st,3)) {
		if (!(sd = map_charid2sd(script_getnum(st,3)))) {
			ShowError("buildin_guild_addmember: Player not found (CID: %d).\n", script_getnum(st,3));
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		if (!(sd = script_rid2sd(st))) {
			script_pushint(st, 0);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	sd->guild_invite = guild_id;
	sd->guild_invite_account = 0;
	script_pushint(st, guild_reply_invite(sd, guild_id, 1));
	clif_colormes(sd->fd, color_table[COLOR_WHITE], g->name);
	return SCRIPT_CMD_SUCCESS;
}

/** Expel player from a guild. Can't expel guild leader.
* If player is online, 'guild_id' and 'account_id' will be ignore.
* Otherwise, it needs 'guild_id' and 'account_id'.
* guild_delmember <char_id>{,"<message>"{,<guild_id>,<account_id>}};
* @return 1 if attempt is success, 0 otherwise
* @author [Cydh]
*/
BUILDIN_FUNC(guild_delmember) {
	int char_id = script_getnum(st,2);
	TBL_PC *sd = map_charid2sd(char_id);

	// Player is online
	if (sd) {
		if (!sd->status.guild_id || !sd->guild) {
			ShowError("buildin_guild_delmember: Player doesn't join in any guild (CID: %d/AID: %d).\n", sd->status.char_id, sd->status.account_id);
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
		//script_pushint(st, guild_leave(sd, sd->status.guild_id, sd->status.account_id, sd->status.char_id, script_hasdata(st, 3) ? script_getstr(st,3) : "", true));
		intif_guild_leave(sd->status.guild_id, sd->status.account_id, sd->status.char_id, 1, script_hasdata(st,3) ? script_getstr(st,3) : "");
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}
	// Player isn't online
	else {
		int guild_id = script_getnum(st,4);
		struct guild *g = NULL;

		if (!(g = guild_search(guild_id))) {
			ShowError("buildin_guild_delmember: Guild not found (Guild ID: %d).\n", guild_id);
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
		else {
			int account_id = script_getnum(st,5);
			unsigned short i;
			if (g->member[0].account_id == account_id && g->member[0].char_id == char_id) {
				ShowError("buildin_guild_delmember: Cannot expel guild leader (GID: %d/AID: %d/CID: %d).\n", g->guild_id, account_id, char_id);
				script_pushint(st, 0);
				return SCRIPT_CMD_FAILURE;
			}
			ARR_FIND(0, MAX_GUILD, i, g->member[i].char_id == char_id);
			if (i >= MAX_GUILD) {
				ShowError("buildin_guild_delmember: Player isn't not a guild member (GID: %d/AID: %d/CID: %d).\n", g->guild_id, account_id, char_id);
				script_pushint(st, 0);
				return SCRIPT_CMD_FAILURE;
			}
			intif_guild_leave(g->guild_id, account_id, char_id, 1, script_hasdata(st,3) ? script_getstr(st,3) : "");
			script_pushint(st, 1);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	script_pushint(st, 0);
	return SCRIPT_CMD_FAILURE;
}

/** Changes guild master, new guild master must be online
* guild_changegm <guild_id>,<char_id>;
* @return 1 if attempt is success, 0 otherwise
* @author [Cydh]
*/
BUILDIN_FUNC(guild_changegm) {
	int guild_id = script_getnum(st, 2);
	int char_id = script_getnum(st, 3);
	struct guild *g = guild_search(guild_id);
	TBL_PC *sd = map_charid2sd(char_id);

	if (!g) {
		ShowError("buildin_guild_changegm: Guild not found (Guild ID: %d).\n", guild_id);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	if (!sd) {
		ShowError("buildin_guild_changegm: Player not found (CID: %d).\n", char_id);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	if (sd->status.guild_id != g->guild_id) {
		ShowError("buildin_guild_changegm: Player must be a guild member.\n");
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	intif_guild_change_gm(guild_id, sd->status.name, strlen(sd->status.name)+1);
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}
