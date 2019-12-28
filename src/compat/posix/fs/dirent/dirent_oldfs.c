/**
 * @file
 * @brief
 *
 * @date 04.02.13
 * @author Alexander Kalmuk
 * @author Anton Kozlov
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <fs/perm.h>
#include <fs/vfs.h>
#include <fs/inode.h>
#include <fs/inode_operation.h>
#include <fs/dcache.h>
#include <mem/objalloc.h>
#include <util/dlist.h>
#include <framework/mod/options.h>

#include <dirent.h>
#include <dirent_impl.h>

#define MAX_DIR_QUANTITY OPTION_GET(NUMBER, dir_quantity)

OBJALLOC_DEF(dir_pool, DIR, MAX_DIR_QUANTITY);

DIR *opendir(const char *path) {
	struct path node_path;
	DIR *d;
	int res;

	if (!strcmp(path, ".")) {
		path = "";//getcwd(cur_path, PATH_MAX);
	}

	res = fs_perm_lookup(path, NULL, &node_path);

	if (res != 0) {
		SET_ERRNO(-res);
		return NULL;
	}

	if (!node_is_directory(node_path.node)) {
		SET_ERRNO(ENOTDIR);
		return NULL;
	}

	if (0 != fs_perm_check(node_path.node, S_IROTH)) {
		SET_ERRNO(EACCES);
		return NULL;
	}

	if (NULL == (d = objalloc(&dir_pool))) {
		SET_ERRNO(ENOMEM);
		return NULL;
	}

	d->path = node_path;
	d->current.d_ino = 0;
	d->dir_contex.fs_ctx = 0;

	slist_init(&d->inodes_list);

	return d;
}

int closedir(DIR *dir) {
	struct inode *node;

	if (NULL == dir) {
		SET_ERRNO(EBADF);
		return -1;
	}

	slist_foreach(node, &dir->inodes_list, dirent_link) {
		inode_del(node);
	}
	objfree(&dir_pool, dir);

	return 0;
}

struct dirent *readdir(DIR *dir) {
	struct path child;
	int res;
	struct inode *node;

	SET_ERRNO(0);

	if (NULL == dir) {
		SET_ERRNO(EBADF);
		return NULL;
	}
	if (dir->path.node->i_ops && dir->path.node->i_ops->iterate) {
		node = inode_new(NULL);
		if (NULL == node) {
			SET_ERRNO(ENOMEM);
			return NULL;
		}
		res = dir->path.node->i_ops->iterate(node, dir->path.node, &dir->dir_contex);
		if (res == -1){
			return NULL;
		}
	} else {
		res = vfs_get_child_next(&dir->path,(struct inode *) (uintptr_t) dir->current.d_ino, &child);
		if (0 != res) {
			return NULL;
		}
		node = child.node;
	}

	strncpy(dir->current.d_name, node->name, DIRENT_DNAME_LEN);
	dir->current.d_ino = (ino_t) (uintptr_t) node;

	node->i_nlink ++;

	slist_add_first_link(&node->dirent_link, &dir->inodes_list);

	return &dir->current;
}
