/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 00:00:00 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/25 00:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_time_in_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL));
}

void	ft_usleep(long long time_in_ms, t_sim *sim)
{
	long long	start;
	int			is_over;

	start = get_time_in_ms();
	while ((get_time_in_ms() - start) < time_in_ms)
	{
		pthread_mutex_lock(&sim->sim_lock);
		is_over = sim->is_simulation_over;
		pthread_mutex_unlock(&sim->sim_lock);
		if (is_over)
			break ;
		usleep(500);
	}
}

void	print_state(t_coder *coder, char *state)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->sim->print_lock);
	pthread_mutex_lock(&coder->sim->sim_lock);
	if (!coder->sim->is_simulation_over)
	{
		timestamp = get_time_in_ms() - coder->sim->start_time;
		printf("%lld %d %s\n", timestamp, coder->id, state);
	}
	pthread_mutex_unlock(&coder->sim->sim_lock);
	pthread_mutex_unlock(&coder->sim->print_lock);
}
