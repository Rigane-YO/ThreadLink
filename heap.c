/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: valrakot <valrakot@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 14:13:25 by valrakot          #+#    #+#             */
/*   Updated: 2026/09/23 14:13:47 by valrakot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_higher_priority(t_heap_node a, t_heap_node b, char *sched)
{
	if (a.key != b.key)
		return (a.key < b.key);
	if (strcmp(sched, "edf") == 0)
		return (a.coder->id < b.coder->id);
	return (0);
}

static void	swap_nodes(t_heap_node *a, t_heap_node *b)
{
	t_heap_node	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	heap_push(t_heap *heap, t_coder *coder, long long key, char *scheduler)
{
	int	i;
	int	parent;

	if (heap->size >= heap->capacity)
		return (1);
	i = heap->size;
	heap->data[i].coder = coder;
	heap->data[i].key = key;
	heap->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (is_higher_priority(heap->data[i], heap->data[parent],
				scheduler))
		{
			swap_nodes(&heap->data[i], &heap->data[parent]);
			i = parent;
		}
		else
			break ;
	}
	return (0);
}

static void	heapify_down(t_heap *heap, char *scheduler)
{
	int	i;
	int	s;
	int	l;

	i = 0;
	while (1)
	{
		s = i;
		l = 2 * i + 1;
		if (l < heap->size && is_higher_priority(heap->data[l],
				heap->data[s], scheduler))
			s = l;
		if (l + 1 < heap->size && is_higher_priority(heap->data[l + 1],
				heap->data[s], scheduler))
			s = l + 1;
		if (s == i)
			break ;
		swap_nodes(&heap->data[i], &heap->data[s]);
		i = s;
	}
}

t_coder	*heap_pop(t_heap *heap, char *scheduler)
{
	t_coder	*top_coder;

	if (heap->size == 0)
		return (NULL);
	top_coder = heap->data[0].coder;
	heap->data[0] = heap->data[heap->size - 1];
	heap->size--;
	heapify_down(heap, scheduler);
	return (top_coder);
}
