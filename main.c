/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:31:06 by valrakot          #+#    #+#             */
/*   Updated: 2026/09/23 14:20:21 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

static void	start_simulation(t_sim *sim)
{
	int			i;
	pthread_t	monitor;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_create(
				&sim->coders[i].thread,
				NULL,
				coder_routine, &sim->coders[i]) != 0)
			return ;
		i++;
	}
	if (pthread_create(&monitor, NULL, monitor_routine, sim) != 0)
		return ;
	pthread_join(monitor, NULL);
	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (parse_args(&sim, argc, argv) != 0)
		return (1);
	if (init_simulation(&sim) != 0)
		return (1);
	start_simulation(&sim);
	clean_simulation(&sim);
	return (0);
}
