/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:30:46 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/24 09:30:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Two coders may both be able to re-claim a dongle at essentially the
** same instant (a self-contained pair releases and re-acquires its own
** two dongles in perfect lockstep, while a neighbour that depends on a
** *different* coder's dongle can miss the window by a few milliseconds
** of scheduling jitter). STARVATION_GRACE_MS gives a higher-priority
** neighbour who is almost ready a short window to catch up instead of
** being skipped over forever by a lower-priority coder who happens to
** be ready first.
*/
# define STARVATION_GRACE_MS 20

static long long	compute_key(t_coder *coder)
{
	if (strcmp(coder->sim->scheduler, "edf") == 0)
		return (coder->last_compile_start + coder->sim->time_to_burnout);
	return (get_time_in_ms());
}

static void	order_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		*first = coder->left_dongle;
		*second = coder->right_dongle;
	}
	else
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

static int	pair_is_ready(t_dongle *first, t_dongle *second, long long now)
{
	return (!first->is_held && now >= first->available_at
		&& !second->is_held && now >= second->available_at);
}

static t_coder	*other_owner(t_dongle *dongle, t_coder *coder)
{
	if (dongle->owner_a == coder)
		return (dongle->owner_b);
	return (dongle->owner_a);
}

/*
** Among the candidates still pending, is there a rival for one of
** "coder"'s two dongles who is strictly more urgent (smaller key,
** i.e. earlier in the already-sorted candidates array) and who will
** become ready within STARVATION_GRACE_MS? If so, the dongle should
** be left for them instead of being handed to "coder" right away.
*/
static int	rival_almost_ready(t_coder *rival, t_dongle *shared,
				long long now)
{
	t_dongle	*r_first;
	t_dongle	*r_second;
	t_dongle	*r_other;

	order_dongles(rival, &r_first, &r_second);
	if (r_first == shared)
		r_other = r_second;
	else
		r_other = r_first;
	return (!r_other->is_held
		&& now + STARVATION_GRACE_MS >= r_other->available_at);
}

static int	blocked_by_rival(t_heap_node *candidates, int rank,
				t_dongle *first, t_dongle *second, long long now)
{
	t_coder	*rival;
	int		i;

	i = 0;
	while (i < rank)
	{
		rival = other_owner(first, candidates[rank].coder);
		if (candidates[i].coder == rival
			&& rival_almost_ready(rival, first, now))
			return (1);
		rival = other_owner(second, candidates[rank].coder);
		if (candidates[i].coder == rival
			&& rival_almost_ready(rival, second, now))
			return (1);
		i++;
	}
	return (0);
}

static void	try_grant_pending(t_sim *sim)
{
	t_heap_node	*cand;
	t_dongle	*first;
	t_dongle	*second;
	long long	now;
	int			n;
	int			i;

	cand = sim->scratch;
	n = 0;
	while (sim->pending.size > 0)
		cand[n++] = sim->pending.data[0], heap_pop(&sim->pending,
			sim->scheduler);
	now = get_time_in_ms();
	i = 0;
	while (i < n)
	{
		order_dongles(cand[i].coder, &first, &second);
		if (pair_is_ready(first, second, now)
			&& !blocked_by_rival(cand, i, first, second, now))
		{
			first->is_held = 1;
			second->is_held = 1;
			cand[i].coder->dongles_granted = 1;
		}
		else
			heap_push(&sim->pending, cand[i].coder, cand[i].key,
				cand[i].seq, sim->scheduler);
		i++;
	}
}

static int	sim_is_over(t_sim *sim)
{
	int	over;

	pthread_mutex_lock(&sim->sim_lock);
	over = sim->is_simulation_over;
	pthread_mutex_unlock(&sim->sim_lock);
	return (over);
}

static int	wait_for_grant(t_coder *coder)
{
	struct timespec	ts;
	long long		now;

	pthread_mutex_lock(&coder->sim->dongle_lock);
	while (!coder->dongles_granted && !sim_is_over(coder->sim))
	{
		now = get_time_in_ms() + 5;
		ts.tv_sec = now / 1000;
		ts.tv_nsec = (now % 1000) * 1000000LL;
		pthread_cond_timedwait(&coder->sim->dongle_cond,
			&coder->sim->dongle_lock, &ts);
		try_grant_pending(coder->sim);
	}
	pthread_mutex_unlock(&coder->sim->dongle_lock);
	return (coder->dongles_granted);
}

int	take_dongles(t_coder *coder)
{
	long long	key;

	pthread_mutex_lock(&coder->sim->dongle_lock);
	key = compute_key(coder);
	coder->dongles_granted = 0;
	heap_push(&coder->sim->pending, coder, key, coder->sim->next_seq++,
		coder->sim->scheduler);
	try_grant_pending(coder->sim);
	pthread_mutex_unlock(&coder->sim->dongle_lock);
	if (!wait_for_grant(coder))
		return (0);
	print_state(coder, "has taken a dongle");
	print_state(coder, "has taken a dongle");
	return (1);
}

void	release_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->sim->dongle_lock);
	coder->left_dongle->is_held = 0;
	coder->left_dongle->available_at
		= get_time_in_ms() + coder->sim->dongle_cooldown;
	coder->right_dongle->is_held = 0;
	coder->right_dongle->available_at
		= get_time_in_ms() + coder->sim->dongle_cooldown;
	try_grant_pending(coder->sim);
	pthread_cond_broadcast(&coder->sim->dongle_cond);
	pthread_mutex_unlock(&coder->sim->dongle_lock);
}
