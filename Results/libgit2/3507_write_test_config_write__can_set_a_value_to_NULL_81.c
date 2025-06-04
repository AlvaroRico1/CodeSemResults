void test_config_write__can_set_a_value_to_NULL(void)
{
    git_repository *repository;
    git_config *config;

    repository = cl_git_sandbox_init("testrepo.git");

    cl_git_pass(git_repository_config(&config, repository));
    cl_git_fail(git_config_set_string(config, "a.b.c", NULL));
    git_config_free(config);

    cl_git_sandbox_cleanup();
}


// Source: write.c
// Lines 465-477
