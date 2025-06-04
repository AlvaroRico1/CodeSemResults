screen_write_rawstring(struct screen_write_ctx *ctx, u_char *str, u_int len)
{
	struct tty_ctx	ttyctx;

	screen_write_initctx(ctx, &ttyctx, 0);
	ttyctx.ptr = str;
	ttyctx.num = len;

	tty_write(tty_cmd_rawstring, &ttyctx);
}


// Source: screen-write.c
// Lines 2091-2100
