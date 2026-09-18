#include "codexion.h"

// Réveille tous les threads bloqués dans acquire_dongle()
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

void	*monitor_routine(void *arg)
{
	t_sim		*sim;
	int			i;
	int			finished_coders;
	long long	current_time;

	sim = (t_sim *)arg;
	while (1)
	{
		finished_coders = 0;
		i = 0;
		while (i < sim->number_of_coders)
		{
			pthread_mutex_lock(&sim->sim_lock);
			current_time = get_time_in_ms();
			
			// 1. Détection du Burnout
			if ((current_time - sim->coders[i].last_compile_start) >= sim->time_to_burnout)
			{
				sim->is_simulation_over = 1;
				pthread_mutex_unlock(&sim->sim_lock);
				
				// Affichage sécurisé et immédiat du décès
				pthread_mutex_lock(&sim->print_lock);
				printf("%lld %d burned out\n", current_time - sim->start_time, sim->coders[i].id);
				pthread_mutex_unlock(&sim->print_lock);
				
				wake_all_coders(sim);
				return (NULL);
			}
			
			// 2. Comptage des compilations terminées
			if (sim->coders[i].compiles_done >= sim->num_compiles_required)
				finished_coders++;
			pthread_mutex_unlock(&sim->sim_lock);
			i++;
		}
		
		// 3. Condition de victoire (Tout le monde a fini)[cite: 1]
		if (finished_coders == sim->number_of_coders)
		{
			pthread_mutex_lock(&sim->sim_lock);
			sim->is_simulation_over = 1;
			pthread_mutex_unlock(&sim->sim_lock);
			
			wake_all_coders(sim);
			return (NULL);
		}
		
		// Pause de 1 ms pour ne pas saturer le processeur, 
		// tout en restant largement en dessous des 10 ms de tolérance imposées[cite: 1].
		usleep(1000); 
	}
	return (NULL);
}