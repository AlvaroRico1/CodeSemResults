cmd_kill_session_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args		*args = cmd_get_args(self);
	struct cmd_find_state	*target = cmdq_get_target(item);
	struct session		*s = target->s, *sloop, *stmp;
	struct winlink		*wl;

	if (args_has(args, 'C')) {
		RB_FOREACH(wl, winlinks, &s->windows) {
			wl->window->flags &= ~WINDOW_ALERTFLAGS;
			wl->flags &= ~WINLINK_ALERTFLAGS;
		}
		server_redraw_session(s);
	} else if (args_has(args, 'a')) {
		RB_FOREACH_SAFE(sloop, sessions, &sessions, stmp) {
			if (sloop != s) {
				server_destroy_session(sloop);
				session_destroy(sloop, 1, __func__);
			}
		}
	} else {
		server_destroy_session(s);
		session_destroy(s, 1, __func__);
	}
	return (CMD_RETURN_NORMAL);
}


// Source: cmd-kill-session.c
// Lines 46-71
