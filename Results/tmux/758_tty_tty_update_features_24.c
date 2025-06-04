tty_update_features(struct tty *tty)
{
	struct client	*c = tty->client;

	if (tty_apply_features(tty->term, c->term_features))
		tty_term_apply_overrides(tty->term);

	if (tty_use_margin(tty))
		tty_putcode(tty, TTYC_ENMG);
	if (options_get_number(global_options, "extended-keys"))
		tty_puts(tty, tty_term_string(tty->term, TTYC_ENEKS));
	if (options_get_number(global_options, "focus-events"))
		tty_puts(tty, tty_term_string(tty->term, TTYC_ENFCS));
	if (tty->term->flags & TERM_VT100LIKE)
		tty_puts(tty, "\033[?7727h");
}


// Source: tty.c
// Lines 458-473
