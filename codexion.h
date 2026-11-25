/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:30:40 by valrakot          #+#    #+#             */
/*   Updated: 2026/09/23 12:35:21 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;
typedef struct s_sim	t_sim;

typedef struct s_heap_node
{
	t_coder		*coder;
	long long	key;
	long long	seq;
}				t_heap_node;

typedef struct s_heap
{
	t_heap_node	*data;
	int			capacity;
	int			size;
}				t_heap;

struct s_dongle
{
	int				id;
	long long		available_at;
	int				is_held;
	struct s_coder	*owner_a;
	struct s_coder	*owner_b;
};

struct s_coder
{
	int				id;
	int				compiles_done;
	long long		last_compile_start;
	int				is_dead;
	pthread_t		thread;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	int				dongles_granted;
	t_sim			*sim;
};

struct s_sim
{
	int				number_of_coders;
	long long		time_to_burnout;
	long long		time_to_compile;
	long long		time_to_debug;
	long long		time_to_refactor;
	int				number_compiles_required;
	long long		dongle_cooldown;
	char			*scheduler;
	long long		start_time;
	int				is_simulation_over;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	sim_lock;
	pthread_mutex_t	dongle_lock;
	pthread_cond_t	dongle_cond;
	t_heap			pending;
	t_heap_node		*scratch;
	long long		next_seq;
	t_dongle		*dongles;
	t_coder			*coders;
};

long long	get_time_in_ms(void);
void		ft_usleep(long long time_in_ms, t_sim *sim);
void		print_state(t_coder *coder, char *state);

int			parse_args(t_sim *sim, int argc, char **argv);
int			init_dongles(t_sim *sim);
int			init_pending(t_sim *sim);
int			init_coders(t_sim *sim);
int			init_simulation(t_sim *sim);
void		clean_simulation(t_sim *sim);

int			heap_push(t_heap *heap, t_heap_node node, char *scheduler);
t_coder		*heap_pop(t_heap *heap, char *scheduler);

/*
** Two coders may both be able to re-claim a dongle at essentially the
** same instant (a self-contained pair releases and re-acquires its
** own two dongles in perfect lockstep, while a neighbour that depends
** on a *different* coder's dongle can miss the window by a few
** milliseconds of scheduling jitter). STARVATION_GRACE_MS gives a
** higher-priority neighbour who is almost ready a short window to
** catch up instead of being skipped over forever by a lower-priority
** coder who happens to be ready first.
*/
# define STARVATION_GRACE_MS 20

long long	compute_key(t_coder *coder);
void		order_dongles(t_coder *coder, t_dongle **first, t_dongle **second);
int			pair_is_ready(t_dongle *first, t_dongle *second, long long now);
t_coder		*other_owner(t_dongle *dongle, t_coder *coder);
int			rival_almost_ready(t_coder *rival, t_dongle *shared, long long now);
void		try_grant_pending(t_sim *sim);
int			wait_for_grant(t_coder *coder);
int			take_dongles(t_coder *coder);
void		release_dongles(t_coder *coder);

void		*coder_routine(void *arg);
void		*monitor_routine(void *arg);

#endif