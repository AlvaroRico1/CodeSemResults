const char *parse_feature_value(const char *feature_list, const char *feature, int *lenp, int *offset)
{
	int len;

	if (!feature_list)
		return NULL;

	len = strlen(feature);
	if (offset)
		feature_list += *offset;
	while (*feature_list) {
		const char *found = strstr(feature_list, feature);
		if (!found)
			return NULL;
		if (feature_list == found || isspace(found[-1])) {
			const char *value = found + len;
			/* feature with no value (e.g., "thin-pack") */
			if (!*value || isspace(*value)) {
				if (lenp)
					*lenp = 0;
				if (offset)
					*offset = found + len - feature_list;
				return value;
			}
			/* feature with a value (e.g., "agent=git/1.2.3") */
			else if (*value == '=') {
				int end;

				value++;
				end = strcspn(value, " \t\n");
				if (lenp)
					*lenp = end;
				if (offset)
					*offset = value + end - feature_list;
				return value;
			}
			/*
			 * otherwise we matched a substring of another feature;
			 * keep looking
			 */
		}


// Source: connect.c
// Lines 540-580
