#ifndef DIRENT_IMPL_H_
#define DIRENT_IMPL_H_

#include <sys/types.h>
#include <sys/cdefs.h>

#include <util/slist.h>

#include <fs/path.h>
#include <fs/dir_context.h>

struct DIR_struct {
	struct dirent current;
	struct path path;
	struct dir_ctx dir_contex;
	struct slist inodes_list;
};

#endif /* DIRENT_IMPL_H_ */
