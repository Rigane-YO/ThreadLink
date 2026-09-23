/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:53:23 by valrakot          #+#    #+#             */
/*   Updated: 2026/09/23 12:53:30 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

static int	is_sim_over(t_coder *coder)
{
	int	over;

	pthread_mutex_lock(&coder->sim->sim_lock);
	over = coder->sim->is_simulation_over;
	pthread_mutex_unlock(&coder->sim->sim_lock);
	return (over);
}

static void	coder_cycle(t_coder *coder)
{
	take_dongles(coder);
	pthread_mutex_lock(&coder->sim->sim_lock);
	coder->last_compile_start = get_time_in_ms();
	pthread_mutex_unlock(&coder->sim->sim_lock);
	print_state(coder, "is compiling");
	ft_usleep(coder->sim->time_to_compile, coder->sim);
	release_dongles(coder);
	pthread_mutex_lock(&coder->sim->sim_lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->sim->sim_lock);
	print_state(coder, "is debugging");
	ft_usleep(coder->sim->time_to_debug, coder->sim);
	print_state(coder, "is refactoring");
	ft_usleep(coder->sim->time_to_refactor, coder->sim);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->id % 2 == 0)
		ft_usleep(coder->sim->time_to_compile / 10, coder->sim);
	while (!is_sim_over(coder))
		coder_cycle(coder);
	return (NULL);
}
