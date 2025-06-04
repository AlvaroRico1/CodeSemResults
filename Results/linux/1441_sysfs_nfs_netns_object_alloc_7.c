static struct kobject *nfs_netns_object_alloc(const char *name,
		struct kset *kset, struct kobject *parent)
{
	struct kobject *kobj;

	kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
	if (kobj) {
		kobj->kset = kset;
		if (kobject_init_and_add(kobj, &nfs_netns_object_type,
					parent, "%s", name) == 0)
			return kobj;
		kobject_put(kobj);
	}
	return NULL;
}


// Source: sysfs.c
// Lines 40-54
