static int file_cb(const git_diff_delta *delta, float progress, void *payload)
{
    int *called = (int *) payload;
    GIT_UNUSED(delta);
    GIT_UNUSED(progress);
    (*called)++;
    return 0;
}


// Source: parse.c
// Lines 273-280
