/*
 *  Burst-Oriented Response Enhancer (BORE) CPU Scheduler
 *  Copyright (C) 2021-2025 Masahito Suzuki <firelzrd@gmail.com>
 */
#include <linux/cpuset.h>
#include <linux/sched/task.h>
#include <linux/sched/bore.h>
#include "sched.h"

#ifdef CONFIG_SCHED_BORE
u8   __read_mostly sched_bore                   = 1;
u8   __read_mostly sched_burst_inherit_type     = 2;
u8   __read_mostly sched_burst_smoothness       = 1;
u8   __read_mostly sched_burst_penalty_offset   = 24;
uint __read_mostly sched_burst_penalty_scale    = 1536;
uint __read_mostly sched_burst_cache_lifetime   = 75000000;
static int __maybe_unused maxval_prio    =   39;

#define MAX_BURST_PENALTY ((40U << 8) - 1)
#define BURST_CACHE_STOP_COUNT 63

static u32 (*inherit_penalty_fn)(struct task_struct *, u64, u64);

static inline u32 log2p1_u64_u32fp(u64 v, u8 fp)
{
	u32 exponent, mantissa;

	if (!v)
		return 0;

	exponent = fls64(v);
	mantissa = (u32)(v << (64 - exponent) << 1 >> (64 - fp));

	return (exponent << fp) | mantissa;
}

static inline u32 calc_burst_penalty(u64 burst_time)
{
	u32 greed, tolerance, penalty, scaled_penalty;

	greed = log2p1_u64_u32fp(burst_time, 8);
	tolerance = sched_burst_penalty_offset << 8;
	penalty = max(0, (s32)(greed - tolerance));
	scaled_penalty = penalty * sched_burst_penalty_scale >> 10;

	return min(MAX_BURST_PENALTY, scaled_penalty);
}

static inline u64 rescale_slice(u64 delta, u8 old_prio, u8 new_prio)
{
	u64 unscaled, rescaled;

	unscaled = mul_u64_u32_shr(delta, sched_prio_to_weight[old_prio], 10);
	rescaled = mul_u64_u32_shr(unscaled, sched_prio_to_wmult[new_prio], 22);

	return rescaled;
}

static inline u32 binary_smooth(u32 new, u32 old)
{
	u32 increment, shift, divisor;

	if (new <= old)
		return new;

	increment = new - old;
	shift = sched_burst_smoothness;
	divisor = 1U << shift;

	return old + ((increment + divisor - 1) >> shift);
}

static void reweight_task_by_prio(struct task_struct *p, int prio)
{
	struct sched_entity *se;
	unsigned long weight;

	if (task_has_idle_policy(p))
		return;

	se = &p->se;
	weight = scale_load(sched_prio_to_weight[prio]);

	if (se->on_rq) {
		p->bore.stop_update = true;
		reweight_entity(cfs_rq_of(se), se, weight);
		p->bore.stop_update = false;
	} else {
		se->load.weight = weight;
	}

	se->load.inv_weight = sched_prio_to_wmult[prio];
}

u8 effective_prio_bore(struct task_struct *p)
{
	int prio = p->static_prio - MAX_RT_PRIO;

	if (likely(sched_bore))
		prio += p->bore.score;

	return (u8)clamp(prio, 0, maxval_prio);
}

static void update_penalty(struct task_struct *p)
{
	struct bore_ctx *ctx = &p->bore;
	u8 prev_prio, new_prio;
	u32 penalty = 0, curr_penalty;

	prev_prio = effective_prio_bore(p);

	if (!(p->flags & PF_KTHREAD)) {
		curr_penalty = ctx->curr_penalty;
		penalty = ctx->prev_penalty;
		if (penalty < curr_penalty)
			penalty = curr_penalty;
	}

	ctx->penalty = penalty;

	new_prio = effective_prio_bore(p);
	if (new_prio != prev_prio)
		reweight_task_by_prio(p, new_prio);
}

void update_curr_bore(struct task_struct *p, u64 delta_exec)
{
	struct bore_ctx *ctx = &p->bore;
	u32 curr_penalty;

	if (ctx->stop_update)
		return;

	ctx->burst_time += delta_exec;
	curr_penalty = calc_burst_penalty(ctx->burst_time);
	ctx->curr_penalty = curr_penalty;

	if (curr_penalty <= ctx->prev_penalty)
		return;

	update_penalty(p);
}

void restart_burst_bore(struct task_struct *p)
{
	struct bore_ctx *ctx = &p->bore;
	u32 new_penalty;

	new_penalty = binary_smooth(ctx->curr_penalty, ctx->prev_penalty);
	ctx->prev_penalty = new_penalty;
	ctx->curr_penalty = 0;
	ctx->burst_time = 0;
	update_penalty(p);
}

void restart_burst_rescale_deadline_bore(struct task_struct *p)
{
	struct sched_entity *se = &p->se;
	s64 vremain, vscaled;
	u8 old_prio, new_prio;

	vremain = se->deadline - se->vruntime;
	old_prio = effective_prio_bore(p);

	restart_burst_bore(p);
	new_prio = effective_prio_bore(p);

	if (old_prio > new_prio) {
		vscaled = rescale_slice(abs(vremain), old_prio, new_prio);
		if (unlikely(vremain < 0))
			vscaled = -vscaled;
		se->deadline = se->vruntime + vscaled;
	}
}

static inline bool task_is_bore_eligible(struct task_struct *p)
{
	return p && p->sched_class == &fair_sched_class && !p->exit_state;
}

#ifndef for_each_child_task
#define for_each_child_task(p, t) \
	list_for_each_entry(t, &(p)->children, sibling)
#endif

static inline u32 count_children_upto2(struct task_struct *p)
{
	struct list_head *head = &p->children;
	struct list_head *next = head->next;

	return (next != head) + (next->next != head);
}

static inline bool burst_cache_expired(struct bore_bc *bc, u64 now)
{
	u64 timestamp = bc->timestamp << BORE_BC_TIMESTAMP_SHIFT;

	return now - timestamp > sched_burst_cache_lifetime;
}

static void update_burst_cache(struct bore_bc *bc,
			       struct task_struct *p, u32 count, u32 total, u64 now)
{
	u32 average = count ? total / count : 0;

	bc->penalty = max_t(u32, average, p->bore.penalty);
	bc->timestamp = now >> BORE_BC_TIMESTAMP_SHIFT;
}

static u32 inherit_none(struct task_struct *parent, u64 clone_flags, u64 now)
{
	return 0;
}

static u32 inherit_from_parent(struct task_struct *parent, u64 clone_flags, u64 now)
{
	struct bore_bc *bc;
	struct task_struct *child;
	u32 count = 0, total = 0;

	if (clone_flags & CLONE_PARENT)
		parent = parent->real_parent;

	bc = &parent->bore.subtree;

	if (burst_cache_expired(bc, now)) {
		for_each_child_task(parent, child) {
			if (count >= BURST_CACHE_STOP_COUNT)
				break;
			if (!task_is_bore_eligible(child))
				continue;
			count++;
			total += child->bore.penalty;
		}

		update_burst_cache(bc, parent, count, total, now);
	}

	return bc->penalty;
}

static u32 inherit_from_ancestor_hub(struct task_struct *parent,
				     u64 clone_flags, u64 now)
{
	struct task_struct *ancestor = parent;
	struct bore_bc *bc;
	u32 sole_child_count = 0, count = 0, total = 0;
	struct task_struct *next, *direct_child, *descendant;

	if (clone_flags & CLONE_PARENT) {
		ancestor = ancestor->real_parent;
		sole_child_count = 1;
	}

	for (; (next = ancestor->real_parent) != ancestor &&
	       count_children_upto2(ancestor) <= sole_child_count;
	     ancestor = next, sole_child_count = 1) {}

	bc = &ancestor->bore.subtree;

	if (burst_cache_expired(bc, now)) {
		for_each_child_task(ancestor, direct_child) {
			if (count >= BURST_CACHE_STOP_COUNT)
				break;

			descendant = direct_child;
			while (count_children_upto2(descendant) == 1)
				descendant = list_first_entry(&descendant->children,
							      struct task_struct, sibling);

			if (!task_is_bore_eligible(descendant))
				continue;
			count++;
			total += descendant->bore.penalty;
		}

		update_burst_cache(bc, ancestor, count, total, now);
	}

	return bc->penalty;
}

static u32 inherit_from_thread_group(struct task_struct *p, u64 now)
{
	struct task_struct *leader, *sibling;
	struct bore_bc *bc;
	u32 count = 0, total = 0;

	leader = p->group_leader;
	bc = &leader->bore.group;

	if (burst_cache_expired(bc, now)) {
		for_each_thread(leader, sibling) {
			if (count >= BURST_CACHE_STOP_COUNT)
				break;
			if (!task_is_bore_eligible(sibling))
				continue;
			count++;
			total += sibling->bore.penalty;
		}

		update_burst_cache(bc, leader, count, total, now);
	}

	return bc->penalty;
}

void task_fork_bore(struct task_struct *p,
		    struct task_struct *parent, u64 clone_flags, u64 now)
{
	struct bore_ctx *ctx;
	u32 inherited_penalty;

	if (!task_is_bore_eligible(p) || unlikely(!sched_bore))
		return;

	ctx = &p->bore;
	inherited_penalty = (clone_flags & CLONE_THREAD) ?
		inherit_from_thread_group(parent, now) :
		inherit_penalty_fn(parent, clone_flags, now);

	if (ctx->prev_penalty < inherited_penalty)
		ctx->prev_penalty = inherited_penalty;

	ctx->curr_penalty  = 0;
	ctx->burst_time    = 0;
	ctx->stop_update   = false;
	ctx->futex_waiting = false;

	update_penalty(p);
}

void reset_task_bore(struct task_struct *p)
{
	memset(&p->bore, 0, sizeof(struct bore_ctx));
}

static void update_inherit_type(void)
{
	switch (sched_burst_inherit_type) {
	case 1:
		inherit_penalty_fn = inherit_from_parent;
		break;
	case 2:
		inherit_penalty_fn = inherit_from_ancestor_hub;
		break;
	default:
		inherit_penalty_fn = inherit_none;
	}
}

void __init sched_init_bore(void)
{
	printk(KERN_INFO "%s %s by %s\n",
	       SCHED_BORE_PROGNAME, SCHED_BORE_VERSION, SCHED_BORE_AUTHOR);

	reset_task_bore(&init_task);
	update_inherit_type();
}

static void readjust_all_task_weights(void)
{
	struct task_struct *task;
	struct rq *rq;
	struct rq_flags rf;

	write_lock_irq(&tasklist_lock);
	for_each_process(task) {
		if (!task_is_bore_eligible(task))
			continue;
		rq = task_rq_lock(task, &rf);
		update_rq_clock(rq);
		reweight_task_by_prio(task, effective_prio_bore(task));
		task_rq_unlock(rq, task, &rf);
	}
	write_unlock_irq(&tasklist_lock);
}

int sched_bore_update_handler(struct ctl_table *table,
			      int write, void __user *buffer,
			      size_t *lenp, loff_t *ppos)
{
	int ret = proc_dou8vec_minmax(table, write, buffer, lenp, ppos);

	if (ret || !write)
		return ret;

	readjust_all_task_weights();
	return 0;
}

int sched_burst_inherit_type_update_handler(struct ctl_table *table,
					    int write, void __user *buffer,
					    size_t *lenp, loff_t *ppos)
{
	int ret = proc_dou8vec_minmax(table, write, buffer, lenp, ppos);

	if (ret || !write)
		return ret;

	update_inherit_type();
	return 0;
}
#endif /* CONFIG_SCHED_BORE */