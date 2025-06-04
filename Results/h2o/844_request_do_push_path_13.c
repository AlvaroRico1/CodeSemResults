static void do_push_path(void *_req, const char *path, size_t path_len, int is_critical)
{
    h2o_req_t *req = _req;

    if (req->conn->callbacks->push_path != NULL)
        req->conn->callbacks->push_path(req, path, path_len, is_critical);
}


// Source: request.c
// Lines 837-843
