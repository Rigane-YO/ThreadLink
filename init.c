/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:31:03 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/25 00:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->dongles[i].id = i + 1;
		sim->dongles[i].available_at = 0;
		sim->dongles[i].is_held = 0;
		i++;
	}
	return (0);
}

int	init_pending(t_sim *sim)
{
	sim->pending.capacity = sim->number_of_coders;
	sim->pending.size = 0;
	sim->pending.data = malloc(sizeof(t_heap_node) * sim->number_of_coders);
	if (!sim->pending.data)
		return (1);
	sim->scratch = malloc(sizeof(t_heap_node) * sim->number_of_coders);
	if (!sim->scratch)
		return (1);
	sim->next_seq = 0;
	return (0);
}

static void	assign_dongles(t_sim *sim, int i)
{
	sim->coders[i].left_dongle = &sim->dongles[i];
	sim->dongles[i].owner_a = &sim->coders[i];
	if (sim->number_of_coders == 1)
		sim->coders[i].right_dongle = &sim->dongles[i];
	else
	{
		sim->coders[i].right_dongle
			= &sim->dongles[(i + 1) % sim->number_of_coders];
		sim->dongles[(i + 1) % sim->number_of_coders].owner_b
			= &sim->coders[i];
	}
}

int	init_coders(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!sim->coders)
		return (1);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].last_compile_start = sim->start_time;
		sim->coders[i].is_dead = 0;
		sim->coders[i].dongles_granted = 0;
		sim->coders[i].sim = sim;
		assign_dongles(sim, i);
		i++;
	}
	return (0);
}
