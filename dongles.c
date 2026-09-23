#include "codexion.h"

static void acquire_dongles(t_coder *coder, t_dongle *dongle)
{
	long long key;

	pthread_mutex_lock(&dongle->mutex);

	if(strcmp(coder->sim->scheduler, "edf") == 0)
		key = coder->last_compile_start + coder->sim->time_to_burnout;
	else
		key = get_time_in_ms();
	heap_push(&dongle->queue, coder, key, coder->sim->scheduler);

	while (1)
	{
		pthread_mutex_lock(&coder->sim->sim_lock);
		if (coder->sim->is_simulation_over)
		{
			pthread_mutex_unlock(&coder->sim->sim_lock);
			break ;
		}

		pthread_mutex_unlock(&coder->sim->sim_lock);

		if (
			dongle->queue.size > 0 && dongle->queue.data[0].coder == coder
			&& get_time_in_ms() >= dongle->available_at
		)
			break;
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	heap_pop(&dongle->queue, coder->sim->scheduler);
	pthread_mutex_unlock(&dongle->mutex);
	print_state(coder, "has taken a dongle");
}

void take_dongles(t_coder *coder)
{
	t_dongle *first;
	t_dongle *second;

	if (coder->sim->number_of_coders == 1)
	{
		acquire_dongles(coder, coder->left_dongle);
		return ;
	}

	if (coder -> left_dongle -> id < coder -> right_dongle -> id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	acquire_dongles(coder, first);
	acquire_dongles(coder, second);
}

static void release_single_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);

	dongle->available_at = get_time_in_ms() + coder->sim->dongle_cooldown;

	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
void release_dongles(t_coder *coder)
{
	release_single_dongle(coder, coder->left_dongle);
	if (coder->sim->number_of_coders > 1)
		release_single_dongle(coder, coder->right_dongle);
}