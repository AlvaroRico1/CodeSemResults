status_prompt_complete_list_menu(struct client *c, char **list, u_int size,
    u_int offset, char flag)
{
	struct menu			*menu;
	struct menu_item		 item;
	struct status_prompt_menu	*spm;
	u_int				 lines = status_line_size(c), height, i;
	u_int				 py;

	if (size <= 1)
		return (0);
	if (c->tty.sy - lines < 3)
		return (0);

	spm = xmalloc(sizeof *spm);
	spm->c = c;
	spm->size = size;
	spm->list = list;
	spm->flag = flag;

	height = c->tty.sy - lines - 2;
	if (height > 10)
		height = 10;
	if (height > size)
		height = size;
	spm->start = size - height;

	menu = menu_create("");
	for (i = spm->start; i < size; i++) {
		item.name = list[i];
		item.key = '0' + (i - spm->start);
		item.command = NULL;
		menu_add_item(menu, &item, NULL, NULL, NULL);
	}

	if (options_get_number(c->session->options, "status-position") == 0)
		py = lines;
	else
		py = c->tty.sy - 3 - height;
	offset += utf8_cstrwidth(c->prompt_string);
	if (offset > 2)
		offset -= 2;
	else
		offset = 0;

	if (menu_display(menu, MENU_NOMOUSE|MENU_TAB, NULL, offset,
	    py, c, NULL, status_prompt_menu_callback, spm) != 0) {
		menu_free(menu);
		free(spm);
		return (0);
	}
	return (1);
}


// Source: status.c
// Lines 1685-1737
