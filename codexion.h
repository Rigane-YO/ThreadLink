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
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	long long		available_at;
	t_heap			queue;
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
	t_dongle		*dongles;
	t_coder			*coders;
};

long long	get_time_in_ms(void);
void		ft_usleep(long long time_in_ms, t_sim *sim);
void		print_state(t_coder *coder, char *state);

int			parse_args(t_sim *sim, int argc, char **argv);
int			init_simulation(t_sim *sim);
void		clean_simulation(t_sim *sim);

int			heap_push(
				t_heap *heap, t_coder *coder, long long key, char *scheduler);
t_coder		*heap_pop(t_heap *heap, char *scheduler);

void		take_dongles(t_coder *coder);
void		release_dongles(t_coder *coder);

void		*coder_routine(void *arg);
void		*monitor_routine(void *arg);

#endif