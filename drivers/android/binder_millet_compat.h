/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ANDROID_BINDER_MILLET_COMPAT_H
#define _ANDROID_BINDER_MILLET_COMPAT_H

/*
 * Whyred uses the 4.4 binder implementation where these bookkeeping
 * structures are private to binder.c.  Millet's binder_gki.c is built as a
 * separate translation unit, so it needs an identical view of the structs.
 * Keep these definitions byte-for-byte compatible with binder.c.
 */

#include <linux/atomic.h>
#include <linux/cred.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <uapi/linux/android/binder.h>
#include <uapi/linux/android/binderfs.h>

#include "binder_alloc.h"
#include "binder_internal.h"

#ifndef BINDER_MILLET_STRUCTS_DEFINED
#define BINDER_MILLET_STRUCTS_DEFINED

enum binder_stat_types {
	BINDER_STAT_PROC,
	BINDER_STAT_THREAD,
	BINDER_STAT_NODE,
	BINDER_STAT_REF,
	BINDER_STAT_DEATH,
	BINDER_STAT_TRANSACTION,
	BINDER_STAT_TRANSACTION_COMPLETE,
	BINDER_STAT_COUNT
};

struct binder_stats {
	atomic_t br[_IOC_NR(BR_ONEWAY_SPAM_SUSPECT) + 1];
	atomic_t bc[_IOC_NR(BC_REPLY_SG) + 1];
	atomic_t obj_created[BINDER_STAT_COUNT];
	atomic_t obj_deleted[BINDER_STAT_COUNT];
};

struct binder_work {
	struct list_head entry;
	enum binder_work_type {
		BINDER_WORK_TRANSACTION = 1,
		BINDER_WORK_TRANSACTION_COMPLETE,
		BINDER_WORK_TRANSACTION_ONEWAY_SPAM_SUSPECT,
		BINDER_WORK_RETURN_ERROR,
		BINDER_WORK_NODE,
		BINDER_WORK_DEAD_BINDER,
		BINDER_WORK_DEAD_BINDER_AND_CLEAR,
		BINDER_WORK_CLEAR_DEATH_NOTIFICATION,
	} type;
};

struct binder_error {
	struct binder_work work;
	uint32_t cmd;
};

struct binder_priority {
	unsigned int sched_policy;
	int prio;
};

struct binder_proc {
	struct hlist_node proc_node;
	struct rb_root threads;
	struct rb_root nodes;
	struct rb_root refs_by_desc;
	struct rb_root refs_by_node;
	struct list_head waiting_threads;
	int pid;
	struct task_struct *tsk;
	const struct cred *cred;
	struct hlist_node deferred_work_node;
	int deferred_work;
	int outstanding_txns;
	bool is_dead;
	bool is_frozen;
	bool sync_recv;
	bool async_recv;
	wait_queue_head_t freeze_wait;

	struct list_head todo;
	struct binder_stats stats;
	struct list_head delivered_death;
	int max_threads;
	int requested_threads;
	int requested_threads_started;
	atomic_t tmp_ref;
	struct binder_priority default_priority;
	struct dentry *debugfs_entry;
	struct binder_alloc alloc;
	struct binder_context *context;
	spinlock_t inner_lock;
	spinlock_t outer_lock;
	struct dentry *binderfs_entry;
	bool oneway_spam_detection_enabled;
};

struct binder_thread {
	struct binder_proc *proc;
	struct rb_node rb_node;
	struct list_head waiting_thread_node;
	int pid;
	int looper;
	bool looper_need_return;
	struct binder_transaction *transaction_stack;
	struct list_head todo;
	bool process_todo;
	struct binder_error return_error;
	struct binder_error reply_error;
	wait_queue_head_t wait;
	struct binder_stats stats;
	atomic_t tmp_ref;
	bool is_dead;
	struct task_struct *task;
};

struct binder_transaction {
	int debug_id;
	struct binder_work work;
	struct binder_thread *from;
	struct binder_transaction *from_parent;
	struct binder_proc *to_proc;
	struct binder_thread *to_thread;
	struct binder_transaction *to_parent;
	unsigned need_reply:1;

	struct binder_buffer *buffer;
	unsigned int code;
	unsigned int flags;
	struct binder_priority priority;
	struct binder_priority saved_priority;
	bool set_priority_called;
	kuid_t sender_euid;
	struct list_head fd_fixups;
	binder_uintptr_t security_ctx;
	spinlock_t lock;
};

#endif /* BINDER_MILLET_STRUCTS_DEFINED */

#endif /* _ANDROID_BINDER_MILLET_COMPAT_H */
