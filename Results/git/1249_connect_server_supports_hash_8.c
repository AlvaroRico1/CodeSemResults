int server_supports_hash(const char *desired, int *feature_supported)
{
	int offset = 0;
	int len;
	const char *hash;

	hash = next_server_feature_value("object-format", &len, &offset);
	if (feature_supported)
		*feature_supported = !!hash;
	if (!hash) {
		hash = hash_algos[GIT_HASH_SHA1].name;
		len = strlen(hash);
	}
	while (hash) {
		if (!xstrncmpz(desired, hash, len))
			return 1;

		hash = next_server_feature_value("object-format", &len, &offset);
	}
	return 0;
}


// Source: connect.c
// Lines 586-606
