/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:30:46 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/24 08:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long long	compute_key(t_coder *coder)
{
	if (strcmp(coder->sim->scheduler, "edf") == 0)
		return (coder->last_compile_start + coder->sim->time_to_burnout);
	return (get_time_in_ms());
}

static void	register_request(t_dongle *dongle, t_coder *coder, long long key)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->queue, coder, key, dongle->next_seq++,
		coder->sim->scheduler);
	pthread_mutex_unlock(&dongle->mutex);
}

static int	dongle_is_ready(t_dongle *dongle, t_coder *coder, long long now)
{
	return (dongle->queue.size > 0 && dongle->queue.data[0].coder == coder
		&& !dongle->is_held && now >= dongle->available_at);
}

static int	take_both_if_ready(t_coder *coder, t_dongle *first,
				t_dongle *second)
{
	long long	now;
	int			ready;

	now = get_time_in_ms();
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	ready = dongle_is_ready(first, coder, now)
		&& dongle_is_ready(second, coder, now);
	if (ready)
	{
		heap_pop(&first->queue, coder->sim->scheduler);
		heap_pop(&second->queue, coder->sim->scheduler);
		first->is_held = 1;
		second->is_held = 1;
	}
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
	return (ready);
}

static int	sim_is_over(t_sim *sim)
{
	int	over;

	pthread_mutex_lock(&sim->sim_lock);
	over = sim->is_simulation_over;
	pthread_mutex_unlock(&sim->sim_lock);
	return (over);
}

static int	wait_for_both(t_coder *coder, t_dongle *first, t_dongle *second)
{
	struct timespec	ts;
	long long		now;
	int				ok;

	ok = 0;
	pthread_mutex_lock(&coder->sim->dongle_lock);
	while (!sim_is_over(coder->sim))
	{
		ok = take_both_if_ready(coder, first, second);
		if (ok)
			break ;
		now = get_time_in_ms() + 5;
		ts.tv_sec = now / 1000;
		ts.tv_nsec = (now % 1000) * 1000000LL;
		pthread_cond_timedwait(&coder->sim->dongle_cond,
			&coder->sim->dongle_lock, &ts);
	}
	pthread_mutex_unlock(&coder->sim->dongle_lock);
	return (ok);
}

int	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long long	key;

	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	key = compute_key(coder);
	register_request(first, coder, key);
	register_request(second, coder, key);
	if (!wait_for_both(coder, first, second))
		return (0);
	print_state(coder, "has taken a dongle");
	print_state(coder, "has taken a dongle");
	return (1);
}

static void	release_single_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_held = 0;
	dongle->available_at = get_time_in_ms() + coder->sim->dongle_cooldown;
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongles(t_coder *coder)
{
	release_single_dongle(coder, coder->left_dongle);
	release_single_dongle(coder, coder->right_dongle);
	pthread_mutex_lock(&coder->sim->dongle_lock);
	pthread_cond_broadcast(&coder->sim->dongle_cond);
	pthread_mutex_unlock(&coder->sim->dongle_lock);
}
