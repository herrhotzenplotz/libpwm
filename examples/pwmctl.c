/* This file is part of libpwm-freebsd.
 *
 * Copyright 2022 Nico Sonack <nsonack@herrhotzenplotz.de>
 * See LICENSE. */

#include <pwm/pwm.h>

#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	int		rc;
	pwm_handle	hdl;
	int		pin;


	if ((rc = pwm_init()) < 0)
		err(EXIT_FAILURE, "pwm_init: %s", pwm_strerror(rc));

	if (argc != 2)
		errx(1, "missing argument");

	pin = strtoul(argv[1], NULL, 10);

	if ((hdl = pwm_open(pin)) < 0)
		err(EXIT_FAILURE, "pwm_open: %s", pwm_strerror(hdl));

	for (;;) {
		char	*endptr		= NULL;
		char     buffer[32]	= {0};
		size_t	 buffer_size	= 0;
		double	 new_percentage = 0;

		if (ferror(stdin) || feof(stdin))
			break;

		fputs("> ", stdout);

		if (!fgets(buffer, sizeof buffer, stdin))
			break;

		buffer_size = strlen(buffer);
		if (buffer[buffer_size - 1] == '\n')
			buffer[--buffer_size] = '\0';

		new_percentage = strtod(buffer, &endptr);
		if (endptr != buffer + buffer_size)
			err(EXIT_FAILURE, "strtod");

		if ((rc = pwm_write(hdl, new_percentage)) < 0)
			errx(EXIT_FAILURE, "pwm_write: %s", pwm_strerror(rc));
	}

	return EXIT_SUCCESS;
}
