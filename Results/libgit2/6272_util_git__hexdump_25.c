void git__hexdump(const char *buffer, size_t len)
{
	static const size_t LINE_WIDTH = 16;

	size_t line_count, last_line, i, j;
	const char *line;

	line_count = (len / LINE_WIDTH);
	last_line = (len % LINE_WIDTH);

	for (i = 0; i < line_count; ++i) {
		printf("%08" PRIxZ "  ", (i * LINE_WIDTH));

		line = buffer + (i * LINE_WIDTH);
		for (j = 0; j < LINE_WIDTH; ++j, ++line) {
			printf("%02x ", (unsigned char)*line & 0xFF);

			if (j == (LINE_WIDTH / 2))
				printf(" ");
		}

		printf(" |");

		line = buffer + (i * LINE_WIDTH);
		for (j = 0; j < LINE_WIDTH; ++j, ++line)
			printf("%c", (*line >= 32 && *line <= 126) ? *line : '.');

		printf("|\n");
	}

	if (last_line > 0) {
		printf("%08" PRIxZ "  ", (line_count * LINE_WIDTH));

		line = buffer + (line_count * LINE_WIDTH);
		for (j = 0; j < last_line; ++j, ++line) {
			printf("%02x ", (unsigned char)*line & 0xFF);

			if (j == (LINE_WIDTH / 2))
				printf(" ");
		}

		if (j < (LINE_WIDTH / 2))
			printf(" ");
		for (j = 0; j < (LINE_WIDTH - last_line); ++j)
			printf("   ");

		printf(" |");

		line = buffer + (line_count * LINE_WIDTH);
		for (j = 0; j < last_line; ++j, ++line)
			printf("%c", (*line >= 32 && *line <= 126) ? *line : '.');

		printf("|\n");
	}

	printf("\n");
}


// Source: util.c
// Lines 371-427
