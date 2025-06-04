static int get_describe_name(struct repository *r,
			     const char *name, int len,
			     struct object_id *oid)
{
	const char *cp;
	unsigned flags = GET_OID_QUIETLY | GET_OID_COMMIT;

	for (cp = name + len - 1; name + 2 <= cp; cp--) {
		char ch = *cp;
		if (!isxdigit(ch)) {
			/* We must be looking at g in "SOMETHING-g"
			 * for it to be describe output.
			 */
			if (ch == 'g' && cp[-1] == '-') {
				cp++;
				len -= cp - name;
				return get_short_oid(r,
						     cp, len, oid, flags);
			}
		}
	}
	return -1;
}


// Source: object-name.c
// Lines 1105-1127
