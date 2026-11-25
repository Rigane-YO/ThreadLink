/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_sync.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 00:00:00 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/25 00:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_sync(t_sim *sim)
{
	if (pthread_mutex_init(&sim->print_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->sim_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->print_lock);
		return (1);
	}
	if (pthread_mutex_init(&sim->dongle_lock, NULL) != 0
		|| pthread_cond_init(&sim->dongle_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->sim_lock);
		pthread_mutex_destroy(&sim->print_lock);
		return (1);
	}
	return (0);
}

static void	destroy_sync(t_sim *sim)
{
	pthread_mutex_destroy(&sim->dongle_lock);
	pthread_cond_destroy(&sim->dongle_cond);
	pthread_mutex_destroy(&sim->sim_lock);
	pthread_mutex_destroy(&sim->print_lock);
}

int	init_simulation(t_sim *sim)
{
	sim->start_time = get_time_in_ms();
	sim->is_simulation_over = 0;
	if (init_sync(sim) != 0)
		return (1);
	if (init_dongles(sim) != 0 || init_pending(sim) != 0
		|| init_coders(sim) != 0)
	{
		destroy_sync(sim);
		return (1);
	}
	return (0);
}
