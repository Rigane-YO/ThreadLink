#include "codexion.h"

// Déclarations des routines
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

static void	start_simulation(t_sim *sim)
{
	int			i;
	pthread_t	monitor;

	i = 0;
	// 1. Création du thread pour chaque codeur[cite: 1]
	while (i < sim->number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine, &sim->coders[i]) != 0)
			return ;
		i++;
	}
	
	// 2. Création du moniteur[cite: 1]
	if (pthread_create(&monitor, NULL, monitor_routine, sim) != 0)
		return ;

	// 3. Attente de la fin du moniteur (il s'arrête seul en cas de fin de simulation)
	pthread_join(monitor, NULL);

	// 4. Attente de la fermeture propre de tous les codeurs
	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_join(sim->coders[i].thread, NULL); //[cite: 1]
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_sim	sim;

	// Étape 1 & 2 : Parsing et initialisation
	if (parse_args(&sim, argc, argv) != 0)
		return (1);
	if (init_simulation(&sim) != 0)
		return (1);
	
	// Étape 3 : Lancement
	start_simulation(&sim);
	
	// Étape 4 : Nettoyage (free et destroy des mutex)
	clean_simulation(&sim);
	
	return (0);
}