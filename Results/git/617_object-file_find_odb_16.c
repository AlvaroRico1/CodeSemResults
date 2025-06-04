struct object_directory *find_odb(struct repository *r, const char *obj_dir)
{
	struct object_directory *odb;
	char *obj_dir_real = real_pathdup(obj_dir, 1);
	struct strbuf odb_path_real = STRBUF_INIT;

	prepare_alt_odb(r);
	for (odb = r->objects->odb; odb; odb = odb->next) {
		strbuf_realpath(&odb_path_real, odb->path, 1);
		if (!strcmp(obj_dir_real, odb_path_real.buf))
			break;
	}

	free(obj_dir_real);
	strbuf_release(&odb_path_real);

	if (!odb)
		die(_("could not find object directory matching %s"), obj_dir);
	return odb;
}


// Source: object-file.c
// Lines 756-775
