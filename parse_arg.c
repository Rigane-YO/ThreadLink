#include "codexion.h"

static long long	ft_atol_check(const char *str)
{
	long long	res;
	int			i;

	i = 0;
	res = 0;
	if (!str || !str[0])
		return (-1);
	if (str[i] == '+')
		i++;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (-1);
		res = res * 10 + (str[i] - '0');
		if (res > 2147483647)
			return (-1);
		i++; // Fix : Incrémentation nécessaire pour éviter la boucle infinie
	}
	return (res);
}

// Fix : Nom harmonisé avec 'parse_args' utilisé dans main.c et codexion.h
int	parse_args(t_sim *sim, int argc, char **argv)
{
	if (argc != 9)
		return (printf("Erreur: Nombre d'arguments incorrect.\n"), 1);

	sim->number_of_coders = (int)ft_atol_check(argv[1]);
	sim->time_to_burnout = ft_atol_check(argv[2]);
	sim->time_to_compile = ft_atol_check(argv[3]);
	sim->time_to_debug = ft_atol_check(argv[4]);
	sim->time_to_refactor = ft_atol_check(argv[5]);
	sim->number_compiles_required = (int)ft_atol_check(argv[6]);
	sim->dongle_cooldown = ft_atol_check(argv[7]);

	if (sim->number_of_coders <= 0 || sim->time_to_burnout <= 0
		|| sim->time_to_compile <= 0 || sim->time_to_debug <= 0
		|| sim->time_to_refactor <= 0 || sim->number_compiles_required <= 0
		|| sim->dongle_cooldown < 0)
		return (printf("Erreur: Argument numérique invalide ou <= 0.\n"), 1);

	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (printf("Erreur: Le scheduler doit être 'fifo' ou 'edf'.\n"), 1);

	sim->scheduler = argv[8];
	return (0);
}