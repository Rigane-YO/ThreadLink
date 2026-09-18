#include "codexion.h"

static int is_higher_priority(
	t_heap_node a, t_heap_node b, char *scheduler
)
{
	if (a.key != b.key)
		return (a.key < b.key);
	if (strcmp(scheduler, "edf") == 0)
		return(a.coder->id < b.coder->id);
	return(0);
}

static void swap_nodes(t_heap_node *a, t_heap_node *b)
{
	t_heap_node tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	heap_push(
	t_heap *heap, t_coder *coder, long long key, char *scheduler
)
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
		parent = (i - 1)/2;
		if (is_higher_priority(heap->data[i], heap->data[parent], scheduler))
		{
			swap_nodes(&heap->data[i], &heap->data[parent]);
			i = parent;
		}
		else
			break;
	}
	return (0);
}

t_coder *heap_pop(t_heap *heap, char *scheduler)
{
	t_coder	*top_coder;
	int		i;
	int		left;
	int		right;
	int		smallest;

	if (heap -> size == 0)
		return (NULL);
	top_coder = heap->data[0].coder;
	heap->data[0] = heap->data[heap->size - 1];
	heap->size--;
	i = 0;
	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (
			left < heap->size && is_higher_priority(
				heap->data[left],
				heap->data[smallest],
				scheduler
			)
		)
			smallest = left;
		if (
			right < heap->size && is_higher_priority(
				heap->data[right],
				heap->data[smallest],
				scheduler
			)
		)
			smallest = right;
		if (smallest != i)
		{
			swap_nodes(&heap->data, &heap->data[smallest]);
			i = smallest;
		}
		else
			break;
	}
	return(top_coder);
}
