/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clean.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:30:33 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/24 07:57:51 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	clean_simulation(t_sim *sim)
{
	if (sim->dongles)
	{
		free(sim->dongles);
		sim->dongles = NULL;
	}
	if (sim->pending.data)
	{
		free(sim->pending.data);
		sim->pending.data = NULL;
	}
	if (sim->scratch)
	{
		free(sim->scratch);
		sim->scratch = NULL;
	}
	if (sim->coders)
	{
		free(sim->coders);
		sim->coders = NULL;
	}
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->sim_lock);
	pthread_mutex_destroy(&sim->dongle_lock);
	pthread_cond_destroy(&sim->dongle_cond);
}
