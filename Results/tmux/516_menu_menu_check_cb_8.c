menu_check_cb(__unused struct client *c, void *data, u_int px, u_int py,
    u_int nx, struct overlay_ranges *r)
{
	struct menu_data	*md = data;
	struct menu		*menu = md->menu;

	server_client_overlay_range(md->px, md->py, menu->width + 4,
	    menu->count + 2, px, py, nx, r);
}


// Source: menu.c
// Lines 167-175
