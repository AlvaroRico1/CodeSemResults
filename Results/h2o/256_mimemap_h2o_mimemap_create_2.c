h2o_mimemap_t *h2o_mimemap_create()
{
    h2o_mimemap_t *mimemap = h2o_mem_alloc_shared(NULL, sizeof(*mimemap), on_dispose);

    mimemap->extmap = kh_init(extmap);
    mimemap->typeset = kh_init(typeset);
    mimemap->default_type = create_extension_type("application/octet-stream", NULL);
    mimemap->num_dynamic = 0;
    on_link(mimemap, mimemap->default_type);

    { /* setup the tiny default */
        static const char *default_types[] = {
#define MIMEMAP(ext, mime) ext, mime,
#include "mimemap/defaults.c.h"
#undef MIMEMAP
            NULL};
        const char **p;
        for (p = default_types; *p != NULL; p += 2)
            h2o_mimemap_define_mimetype(mimemap, p[0], p[1], NULL);
    }


// Source: mimemap.c
// Lines 161-180
