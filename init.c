#include "codexion.h"

long long get_rime_in_ms(void)
{
	struct timeval tv;
		gettimeofday(&tv, NULL);
	return((tv.tv_sec * 1000LL) + (tv.tv_sec/1000LL));
}

static int	init_dongles(t_sim *sim)
{
	int	i;
	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if(!sim->dongles)
		return(1);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->dongles[i].id = i + 1;
		sim->dongles[i].available_at = 0;
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
		{
			while (i > 0)
			{
				i--;
				pthread_mutex_destroy(&sim->dongles[i].mutex);
			}
			free(sim->dongles);
			sim->dongles = NULL;
			return (1);
		}
		i++;
	}
	return (0);
}

static void assign_dongles(t_sim *sim, int i)
{
	sim->coders[i].left_dongle = &sim->dongles[i];
	if (sim->number_of_coders == 1)
		sim->coders[i].right_dongle = &sim->dongles[i];
	else
		sim->coders[i].right_dongle =
				&sim->dongles[(i + 1) % sim->number_of_coders];
}

static int init_coders(t_sim *sim)
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
		sim->coders[i].sim = 0;
		assign_dongles(sim, i);
		i++;
	}
	return 0;
}

int init_simulation(t_sim *sim)
{
	sim->start_time = get_time_im_ms();
	sim->is_simulation_over = 0;
	if (pthread_mutex_init(&sim->print_lock, NULL) != 0)
		return(1);
	if (pthread_mutex_init(&sim->sim_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->sim_lock);
		pthread_mutex_destroy(&sim->print_lock);
		return (1);
	}
	if (init_coders(sim) != 0)
	{
		pthread_mutex_destroy(&sim->sim_lock);
		pthread_mutex_destroy(&sim->print_lock);
		return (1);
	}
	return(0);
}
