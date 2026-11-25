/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:53:23 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/25 00:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
	if (!take_dongles(coder))
		return ;
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

static void	wait_until_over(t_coder *coder)
{
	while (!is_sim_over(coder))
		usleep(500);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->sim->number_of_coders == 1)
	{
		wait_until_over(coder);
		return (NULL);
	}
	if (coder->id % 2 == 0)
		ft_usleep(coder->sim->time_to_compile / 10, coder->sim);
	while (!is_sim_over(coder))
		coder_cycle(coder);
	return (NULL);
}
