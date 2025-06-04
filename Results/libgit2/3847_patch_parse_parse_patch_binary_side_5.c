static int parse_patch_binary_side(
	git_diff_binary_file *binary,
	git_patch_parse_ctx *ctx)
{
	git_diff_binary_t type = GIT_DIFF_BINARY_NONE;
	git_str base85 = GIT_STR_INIT, decoded = GIT_STR_INIT;
	int64_t len;
	int error = 0;

	if (git_parse_ctx_contains_s(&ctx->parse_ctx, "literal ")) {
		type = GIT_DIFF_BINARY_LITERAL;
		git_parse_advance_chars(&ctx->parse_ctx, 8);
	} else if (git_parse_ctx_contains_s(&ctx->parse_ctx, "delta ")) {
		type = GIT_DIFF_BINARY_DELTA;
		git_parse_advance_chars(&ctx->parse_ctx, 6);
	} else {
		error = git_parse_err(
			"unknown binary delta type at line %"PRIuZ, ctx->parse_ctx.line_num);
		goto done;
	}

	if (git_parse_advance_digit(&len, &ctx->parse_ctx, 10) < 0 ||
	    git_parse_advance_nl(&ctx->parse_ctx) < 0 || len < 0) {
		error = git_parse_err("invalid binary size at line %"PRIuZ, ctx->parse_ctx.line_num);
		goto done;
	}

	while (ctx->parse_ctx.line_len) {
		char c;
		size_t encoded_len, decoded_len = 0, decoded_orig = decoded.size;

		git_parse_peek(&c, &ctx->parse_ctx, 0);

		if (c == '\n')
			break;
		else if (c >= 'A' && c <= 'Z')
			decoded_len = c - 'A' + 1;
		else if (c >= 'a' && c <= 'z')
			decoded_len = c - 'a' + (('z' - 'a') + 1) + 1;

		if (!decoded_len) {
			error = git_parse_err("invalid binary length at line %"PRIuZ, ctx->parse_ctx.line_num);
			goto done;
		}

		git_parse_advance_chars(&ctx->parse_ctx, 1);

		encoded_len = ((decoded_len / 4) + !!(decoded_len % 4)) * 5;

		if (!encoded_len || !ctx->parse_ctx.line_len || encoded_len > ctx->parse_ctx.line_len - 1) {
			error = git_parse_err("truncated binary data at line %"PRIuZ, ctx->parse_ctx.line_num);
			goto done;
		}

		if ((error = git_str_decode_base85(
			&decoded, ctx->parse_ctx.line, encoded_len, decoded_len)) < 0)
			goto done;

		if (decoded.size - decoded_orig != decoded_len) {
			error = git_parse_err("truncated binary data at line %"PRIuZ, ctx->parse_ctx.line_num);
			goto done;
		}

		git_parse_advance_chars(&ctx->parse_ctx, encoded_len);

		if (git_parse_advance_nl(&ctx->parse_ctx) < 0) {
			error = git_parse_err("trailing data at line %"PRIuZ, ctx->parse_ctx.line_num);
			goto done;
		}
	}

	binary->type = type;
	binary->inflatedlen = (size_t)len;
	binary->datalen = decoded.size;
	binary->data = git_str_detach(&decoded);

done:
	git_str_dispose(&base85);
	git_str_dispose(&decoded);
	return error;
}


// Source: patch_parse.c
// Lines 764-844
