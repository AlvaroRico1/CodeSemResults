static inline bool read_string_ascii(
	char **out,
	ntlm_client *ntlm,
	ntlm_buf *message,
	uint8_t string_len)
{
	char *str;

	if ((str = malloc(string_len + 1)) == NULL) {
		ntlm_client_set_errmsg(ntlm, "out of memory");
		return false;
	}

	memcpy(str, &message->buf[message->pos], string_len);
	str[string_len] = '\0';

	message->pos += string_len;

	*out = str;
	return true;
}


// Source: ntlm.c
// Lines 507-527
