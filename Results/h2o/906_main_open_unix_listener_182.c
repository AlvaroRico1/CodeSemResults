static int open_unix_listener(h2o_configurator_command_t *cmd, yoml_t *node, struct sockaddr_un *sa, yoml_t **owner_node,
                              yoml_t **group_node, yoml_t **permission_node)
{
    struct stat st;
    int fd = -1;
    struct passwd *owner = NULL, pwbuf;
    char pwbuf_buf[65536];
    gid_t owner_gid = -1;
    unsigned mode = UINT_MAX;

    /* obtain owner and permission */
    if (owner_node == NULL && group_node != NULL) {
        h2o_configurator_errprintf(cmd, *group_node, "`group` cannot be used without `owner`");
        goto ErrorExit;
    }
    if (owner_node != NULL) {
        int r = getpwnam_r((*owner_node)->data.scalar, &pwbuf, pwbuf_buf, sizeof(pwbuf_buf), &owner);
        if (r != 0 || owner == NULL) {
            h2o_configurator_errprintf(cmd, *owner_node, "failed to obtain uid of user:%s: %s", (*owner_node)->data.scalar,
                                       (r == 0 ? "Not found" : strerror(r)));
            goto ErrorExit;
        }
        owner_gid = owner->pw_gid;
        if (group_node != NULL) {
            struct group *group = NULL, grbuf;
            char grbuf_buf[65536];
            r = getgrnam_r((*group_node)->data.scalar, &grbuf, grbuf_buf, sizeof(grbuf_buf), &group);
            if (r != 0 || group == NULL) {
                h2o_configurator_errprintf(cmd, *group_node, "failed to obtain gid of group:%s: %s", (*group_node)->data.scalar,
                                           (r == 0 ? "Not found" : strerror(r)));
                goto ErrorExit;
            }
            owner_gid = group->gr_gid;
        }


// Source: main.c
// Lines 1519-1552
