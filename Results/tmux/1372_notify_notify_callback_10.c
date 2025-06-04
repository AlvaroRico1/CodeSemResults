notify_callback(struct cmdq_item *item, void *data)
{
	struct notify_entry	*ne = data;

	log_debug("%s: %s", __func__, ne->name);

	if (strcmp(ne->name, "pane-mode-changed") == 0)
		control_notify_pane_mode_changed(ne->pane);
	if (strcmp(ne->name, "window-layout-changed") == 0)
		control_notify_window_layout_changed(ne->window);
	if (strcmp(ne->name, "window-pane-changed") == 0)
		control_notify_window_pane_changed(ne->window);
	if (strcmp(ne->name, "window-unlinked") == 0)
		control_notify_window_unlinked(ne->session, ne->window);
	if (strcmp(ne->name, "window-linked") == 0)
		control_notify_window_linked(ne->session, ne->window);
	if (strcmp(ne->name, "window-renamed") == 0)
		control_notify_window_renamed(ne->window);
	if (strcmp(ne->name, "client-session-changed") == 0)
		control_notify_client_session_changed(ne->client);
	if (strcmp(ne->name, "client-detached") == 0)
		control_notify_client_detached(ne->client);
	if (strcmp(ne->name, "session-renamed") == 0)
		control_notify_session_renamed(ne->session);
	if (strcmp(ne->name, "session-created") == 0)
		control_notify_session_created(ne->session);
	if (strcmp(ne->name, "session-closed") == 0)
		control_notify_session_closed(ne->session);
	if (strcmp(ne->name, "session-window-changed") == 0)
		control_notify_session_window_changed(ne->session);

	notify_insert_hook(item, ne);

	if (ne->client != NULL)
		server_client_unref(ne->client);
	if (ne->session != NULL)
		session_remove_ref(ne->session, __func__);
	if (ne->window != NULL)
		window_remove_ref(ne->window, __func__);

	if (ne->fs.s != NULL)
		session_remove_ref(ne->fs.s, __func__);

	format_free(ne->formats);
	free((void *)ne->name);
	free(ne);

	return (CMD_RETURN_NORMAL);
}


// Source: notify.c
// Lines 89-137
