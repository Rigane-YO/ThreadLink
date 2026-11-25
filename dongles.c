/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 12:30:46 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/25 00:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	take_dongles(t_coder *coder)
{
	t_heap_node	node;

	pthread_mutex_lock(&coder->sim->dongle_lock);
	node.coder = coder;
	node.key = compute_key(coder);
	node.seq = coder->sim->next_seq++;
	coder->dongles_granted = 0;
	heap_push(&coder->sim->pending, node, coder->sim->scheduler);
	try_grant_pending(coder->sim);
	pthread_mutex_unlock(&coder->sim->dongle_lock);
	if (!wait_for_grant(coder))
		return (0);
	print_state(coder, "has taken a dongle");
	print_state(coder, "has taken a dongle");
	return (1);
}

void	release_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->sim->dongle_lock);
	coder->left_dongle->is_held = 0;
	coder->left_dongle->available_at
		= get_time_in_ms() + coder->sim->dongle_cooldown;
	coder->right_dongle->is_held = 0;
	coder->right_dongle->available_at
		= get_time_in_ms() + coder->sim->dongle_cooldown;
	try_grant_pending(coder->sim);
	pthread_cond_broadcast(&coder->sim->dongle_cond);
	pthread_mutex_unlock(&coder->sim->dongle_lock);
}
