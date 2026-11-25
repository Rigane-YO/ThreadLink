/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_key.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 00:00:00 by valrakot          #+#    #+#             */
/*   Updated: 2026/11/25 00:00:00 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	compute_key(t_coder *coder)
{
	if (strcmp(coder->sim->scheduler, "edf") == 0)
		return (coder->last_compile_start + coder->sim->time_to_burnout);
	return (get_time_in_ms());
}

void	order_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		*first = coder->left_dongle;
		*second = coder->right_dongle;
	}
	else
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

int	pair_is_ready(t_dongle *first, t_dongle *second, long long now)
{
	return (!first->is_held && now >= first->available_at
		&& !second->is_held && now >= second->available_at);
}

t_coder	*other_owner(t_dongle *dongle, t_coder *coder)
{
	if (dongle->owner_a == coder)
		return (dongle->owner_b);
	return (dongle->owner_a);
}

/*
** Is "rival" (who shares dongle "shared" with some other coder) about
** to become ready on its *other* dongle, within STARVATION_GRACE_MS?
** Used to decide whether a free dongle should be held back briefly
** for a higher-priority neighbour instead of granted immediately.
*/
int	rival_almost_ready(t_coder *rival, t_dongle *shared, long long now)
{
	t_dongle	*r_first;
	t_dongle	*r_second;
	t_dongle	*r_other;

	order_dongles(rival, &r_first, &r_second);
	if (r_first == shared)
		r_other = r_second;
	else
		r_other = r_first;
	return (!r_other->is_held
		&& now + STARVATION_GRACE_MS >= r_other->available_at);
}
