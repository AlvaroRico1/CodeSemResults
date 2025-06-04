menu_mode_cb(__unused struct client *c, void *data, __unused u_int *cx,
    __unused u_int *cy)
{
	struct menu_data	*md = data;

	return (&md->s);
}


// Source: menu.c
// Lines 157-163
