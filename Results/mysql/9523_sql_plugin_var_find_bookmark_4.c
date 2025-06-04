st_bookmark *find_bookmark(const char *plugin, const char *name, int flags) {
  size_t namelen, length, pluginlen = 0;
  char *varname, *p;

  if (!(flags & PLUGIN_VAR_THDLOCAL)) return nullptr;

  namelen = strlen(name);
  if (plugin) pluginlen = strlen(plugin) + 1;
  length = namelen + pluginlen + 2;
  varname = (char *)my_alloca(length);

  if (plugin) {
    strxmov(varname + 1, plugin, "_", name, NullS);
    for (p = varname + 1; *p; p++)
      if (*p == '-') *p = '_';
  } else
    memcpy(varname + 1, name, namelen + 1);

  varname[0] = flags & PLUGIN_VAR_TYPEMASK;

  const auto it = get_bookmark_hash()->find(std::string(varname, length - 1));
  if (it == get_bookmark_hash()->end())
    return nullptr;
  else
    return it->second;
}


// Source: sql_plugin_var.cc
// Lines 796-821
