static void setup_pathconf(h2o_req_t *req, h2o_hostconf_t *hostconf)
{
    h2o_pathconf_t *selected_pathconf = &hostconf->fallback_path;
    size_t i;

    /* setup pathconf, or redirect to "path/" */
    for (i = 0; i != hostconf->paths.size; ++i) {
        h2o_pathconf_t *candidate = hostconf->paths.entries[i];
        if (req->path_normalized.len >= candidate->path.len &&
            memcmp(req->path_normalized.base, candidate->path.base, candidate->path.len) == 0 &&
            (candidate->path.base[candidate->path.len - 1] == '/' || req->path_normalized.len == candidate->path.len ||
             req->path_normalized.base[candidate->path.len] == '/')) {
            selected_pathconf = candidate;
            break;
        }
    }


// Source: request.c
// Lines 172-187
