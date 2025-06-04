static void backend_internal_free(backend_internal *internal)
{
	git_config_backend *backend;

	backend = internal->backend;
	backend->free(backend);
	git__free(internal);
}


// Source: config.c
// Lines 40-47
