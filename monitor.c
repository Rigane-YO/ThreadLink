#include "codexion.h"

static void wake_all_coders(t_sim *sim)
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

void *monior_routine(void *arg)
{
	t_sim		*sim;
	int			i;
	int			finished_coders;
	long long	cureent_time;

	sim = (t_sim *)arg;
	while (1)
	{
		finished_coders = 0;
		i = 0;
		while (i < sim->number_of_coders)
		{
			pthread_mutex_lock(&sim->sim_lock);
			cureent_time = get_time_in_ms();
			if (
				(cureent_time - sim->coders[i].last_compile_start) >=
				sim->time_to_burnout
			)
			{
				sim->is_simulation_over = 1;
				pthread_mutex_unlock(&sim->sim_lock);
				pthread_mutex_lock(&sim->print_lock);
				printf(
					"%lld %d burned out\n", cureent_time - sim->start_time,
					sim->coders[i].id
				);
				pthread_mutex_unlock(&sim->print_lock);
				wake_all_coders(sim);
				return (NULL);
			}
			if (
				sim->coders[i].compiles_done >=
				sim->number_compiles_required
			)
				finished_coders++;
			pthread_mutex_unlock(&sim->sim_lock);
			i++;
		}
		if (finished_coders == sim->number_of_coders)
		{
			pthread_mutex_lock(&sim->sim_lock);
			sim->is_simulation_over = 1;
			pthread_mutex_unlock(&sim->sim_lock);

			wake_all_coders(sim);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}