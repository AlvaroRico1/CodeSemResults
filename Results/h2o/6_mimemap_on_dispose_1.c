static void on_dispose(void *_mimemap)
{
    h2o_mimemap_t *mimemap = _mimemap;
    const char *ext;
    h2o_mimemap_type_t *type;

    kh_destroy(typeset, mimemap->typeset);
    kh_foreach(mimemap->extmap, ext, type, {
        h2o_mem_release_shared((char *)ext);
        h2o_mem_release_shared(type);
    });
    kh_destroy(extmap, mimemap->extmap);
    h2o_mem_release_shared(mimemap->default_type);
}


// Source: mimemap.c
// Lines 64-77
