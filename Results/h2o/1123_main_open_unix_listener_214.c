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
    }
    if (permission_node != NULL && h2o_configurator_scanf(cmd, *permission_node, "%o", &mode) != 0) {
        h2o_configurator_errprintf(cmd, *permission_node, "`permission` must be an octal number");
        goto ErrorExit;
    }

    /* remove existing socket file as suggested in #45 */
    if (lstat(sa->sun_path, &st) == 0) {
        if (S_ISSOCK(st.st_mode)) {
            unlink(sa->sun_path);
        } else {
            h2o_configurator_errprintf(cmd, node, "path:%s already exists and is not an unix socket.", sa->sun_path);
            goto ErrorExit;
        }
    }

    /* add new listener */
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 || bind(fd, (void *)sa, sizeof(*sa)) != 0 || listen(fd, H2O_SOMAXCONN) != 0) {
        h2o_configurator_errprintf(NULL, node, "failed to listen to socket:%s: %s", sa->sun_path, strerror(errno));
        goto ErrorExit;
    }
    set_cloexec(fd);

    /* set file owner and permission */
    if (owner != NULL && chown(sa->sun_path, owner->pw_uid, owner_gid) != 0) {
        h2o_configurator_errprintf(NULL, node, "failed to chown socket:%s to %s (gid %u): %s", sa->sun_path, owner->pw_name,
                                   (unsigned)owner_gid, strerror(errno));
        goto ErrorExit;
    }
    if (mode != UINT_MAX && chmod(sa->sun_path, mode) != 0) {
        h2o_configurator_errprintf(NULL, node, "failed to chmod socket:%s to %o: %s", sa->sun_path, mode, strerror(errno));
        goto ErrorExit;
    }

    return fd;

ErrorExit:
    if (fd != -1)
        close(fd);
    return -1;
}


// Source: main.c
// Lines 1519-1593
