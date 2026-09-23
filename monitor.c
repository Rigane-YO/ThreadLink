/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 14:25:59 by valrakot          #+#    #+#             */
/*   Updated: 2026/09/23 14:26:23 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wake_all_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static int	handle_burnout(t_sim *sim, int id, long long now)
{
	pthread_mutex_lock(&sim->print_lock);
	printf("%lld %d burned out\n", now - sim->start_time, id);
	pthread_mutex_unlock(&sim->print_lock);
	wake_all_coders(sim);
	return (1);
}

static int	check_coder(t_sim *sim, int i, int *finished)
{
	long long	now;
	int			burned;

	burned = 0;
	pthread_mutex_lock(&sim->sim_lock);
	now = get_time_in_ms();
	if ((now - sim->coders[i].last_compile_start) >= sim->time_to_burnout)
	{
		sim->is_simulation_over = 1;
		burned = 1;
	}
	else if (sim->coders[i].compiles_done >= sim->number_compiles_required)
		(*finished)++;
	pthread_mutex_unlock(&sim->sim_lock);
	if (burned)
		return (handle_burnout(sim, sim->coders[i].id, now));
	return (0);
}

static int	check_all_coders(t_sim *sim)
{
	int	i;
	int	finished;

	finished = 0;
	i = 0;
	while (i < sim->number_of_coders)
	{
		if (check_coder(sim, i, &finished))
			return (1);
		i++;
	}
	if (finished == sim->number_of_coders)
	{
		pthread_mutex_lock(&sim->sim_lock);
		sim->is_simulation_over = 1;
		pthread_mutex_unlock(&sim->sim_lock);
		wake_all_coders(sim);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		if (check_all_coders(sim))
			break ;
		usleep(1000);
	}
	return (NULL);
}
