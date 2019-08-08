// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
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

/**
* clearcart({<char_id>});
**/
BUILDIN_FUNC(clearcart)
{
	TBL_PC *sd;
	int i;

	if (!script_charid2sd(2, sd))
		return SCRIPT_CMD_FAILURE;

	for (i = 0; i < MAX_CART; i++) {
		if (sd->cart.u.items_cart[i].amount) {
			pc_cart_delitem(sd, i, sd->cart.u.items_cart[i].amount, 0, LOG_TYPE_SCRIPT);
		}
	}
	return SCRIPT_CMD_SUCCESS;
}
