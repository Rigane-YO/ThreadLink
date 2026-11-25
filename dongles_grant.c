/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_grant.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 00:00:00 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/25 00:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	blocked_by_rival(t_heap_node *candidates, int rank, long long now)
{
	t_dongle	*first;
	t_dongle	*second;
	t_coder		*rival;
	int			i;

	order_dongles(candidates[rank].coder, &first, &second);
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

static void	grant_or_requeue(t_sim *sim, t_heap_node *cand, int rank,
				long long now)
{
	t_dongle	*first;
	t_dongle	*second;

	order_dongles(cand[rank].coder, &first, &second);
	if (pair_is_ready(first, second, now)
		&& !blocked_by_rival(cand, rank, now))
	{
		first->is_held = 1;
		second->is_held = 1;
		cand[rank].coder->dongles_granted = 1;
	}
	else
		heap_push(&sim->pending, cand[rank], sim->scheduler);
}

void	try_grant_pending(t_sim *sim)
{
	t_heap_node	*cand;
	long long	now;
	int			n;
	int			i;

	cand = sim->scratch;
	n = 0;
	while (sim->pending.size > 0)
	{
		cand[n] = sim->pending.data[0];
		heap_pop(&sim->pending, sim->scheduler);
		n++;
	}
	now = get_time_in_ms();
	i = 0;
	while (i < n)
	{
		grant_or_requeue(sim, cand, i, now);
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

int	wait_for_grant(t_coder *coder)
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
