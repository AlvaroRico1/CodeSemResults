static int resolve_to(struct image *image, const struct object_id *result_id)
{
	unsigned long size;
	enum object_type type;

	clear_image(image);

	image->buf = read_object_file(result_id, &type, &size);
	if (!image->buf || type != OBJ_BLOB)
		die("unable to read blob object %s", oid_to_hex(result_id));
	image->len = size;

	return 0;
}


// Source: apply.c
// Lines 3471-3484
