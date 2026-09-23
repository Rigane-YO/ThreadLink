#include "codexion.h"

void	print_state(t_coder *coder, char *state)
{
	long long timestap;
	pthread_mutex_lock(&coder->sim->print_lock);
	pthread_mutex_lock(&coder->sim->sim_lock);
	if (!coder->sim->is_simulation_over)
	{
		timestap = get_time_in_ms() - coder->sim->start_time;
		printf("%lld %d %s\n", timestap, coder->id, state);
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
			break;
		usleep(500);
	}
}

void	*coder_routine(void *arg)
{
	t_coder *coder;
	coder = (t_coder *)arg;

	if (coder->id % 2 == 0)
		ft_usleep(coder->sim->time_to_compile / 10, coder->sim);
	while (1)
	{
		pthread_mutex_lock(&coder->sim->sim_lock);
		if (coder->sim->is_simulation_over)
		{
			pthread_mutex_unlock(&coder->sim->sim_lock);
			break ;
		}
		pthread_mutex_unlock(&coder->sim->sim_lock);

		pthread_mutex_lock(&coder->sim->sim_lock);
		coder->last_compile_start = get_time_in_ms();
		pthread_mutex_unlock(&coder->sim->sim_lock);
		print_state(coder, "is compiling");
		ft_usleep(coder->sim->time_to_compile, coder->sim);

		coder->compiles_done++;
		print_state(coder, "is_debugging");
		ft_usleep(coder->sim->time_to_debug, coder->sim);
		print_state(coder, "is refactoring");
		ft_usleep(coder->sim->time_to_refactor, coder->sim);
	
	}
	return (NULL);
}