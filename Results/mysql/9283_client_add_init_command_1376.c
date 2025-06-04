static int add_init_command(struct st_mysql_options *options, const char *cmd) {
  char *tmp;

  if (!options->init_commands) {
    void *rawmem = my_malloc(key_memory_mysql_options,
                             sizeof(Init_commands_array), MYF(MY_WME));
    if (!rawmem) return 1;
    options->init_commands =
        new (rawmem) Init_commands_array(key_memory_mysql_options);
  }


// Source: client.cc
// Lines 2013-2022
