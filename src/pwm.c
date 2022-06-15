/*
 * Copyright 2022 Nico Sonack <nsonack@herrhotzenplotz.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <pwm/pwm.h>

#include <stdint.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/sysctl.h>


#define MAX_PWM_HANDLES 4

/* Structure backing a pwm handle */
struct pwm_data {
    int		mib[CTL_MAXNAME];
    size_t	mib_len;
    int		used;
    uint32_t    period;
};

/* List of handles */
static struct pwm_data	hdls[MAX_PWM_HANDLES] = {0};
static int		hdls_size = 0;

/* Error code strings */
static const char *error_codes[] = {
    [ PWME_OK]        = "No error",
    [-PWME_NOTPWMPIN] = "Not a PWM pin ",
    [-PWME_BUSY]      = "Pin is already in use",
    [-PWME_SYSCTLWR]  = "sysctl write failure",
    [-PWME_SYSCTLRD]  = "sysctl read failure",
    [-PWME_BADH]      = "bad PWM handle",
    [-PWME_INVAL]     = "invalid argument",
};

/* Lookup table for the pins and their sysctl nodes.
 *
 * TODO: Only pin gpio pin 18 seems to be working correctly. */
static const struct pin_config {
    int			pin;
    int                 handle;
    const char *const	mode;
    const char *const	period;
    const char *const	ratio;
} pin_configs[] = {
    { .pin    = 18, /* Physical pin 12 */
      .handle = 0,
      .mode   = "dev.pwm.0.mode",
      .period = "dev.pwm.0.period",
      .ratio  = "dev.pwm.0.ratio" },
    { .pin    = 13, /* Physical pin 33 */
      .handle = 2,
      .mode   = "dev.pwm.1.mode",
      .period = "dev.pwm.1.period",
      .ratio  = "dev.pwm.1.ratio" },
    { .pin    = 12, /* Physical pin 32 */
      .handle = 1,
      .mode   = "dev.pwm.0.mode2",
      .period = "dev.pwm.0.period2",
      .ratio  = "dev.pwm.0.ratio2" },
    { .pin    = 19, /* Physical pin 35 */
      .handle = 3,
      .mode   = "dev.pwm.1.mode2",
      .period = "dev.pwm.1.period2",
      .ratio  = "dev.pwm.1.ratio2" },
};

const char *
pwm_strerror(int code)
{
    return error_codes[-code];
}

int
pwm_init(void)
{
    return PWME_OK;
}

int
pwm_open(int pin)
{
    int				 rc  = 0;
    int				 hdl = -1;
    const struct pin_config	*cfg = NULL;

    /* Search for the pin config */
    for (int i = 0; i < (sizeof(pin_configs) / sizeof(*pin_configs)); ++i) {
	if (pin_configs[i].pin == pin) {
	    hdl = pin_configs[i].handle;
	    cfg = &pin_configs[i];
	    break;
	}
    }

    /* We couldn't find a config for that pin */
    if (hdl < 0)
	return PWME_NOTPWMPIN;

    /* The pin is already in use */
    if (hdls[hdl].used)
	return PWME_BUSY;

    /* Set the mode */
    int new_mode = 1; /* PWM mode */
    if (sysctlbyname(cfg->mode, NULL, NULL, &new_mode, sizeof(new_mode)) < 0)
	return PWME_SYSCTLWR;

    /* Read the period */
    size_t period_size = sizeof(hdls[hdl].period);
    if (sysctlbyname(cfg->period, &hdls[hdl].period, &period_size, NULL, 0) < 0)
	return PWME_SYSCTLRD;

    /* Get the MIB for the ratio sysctl */
    hdls[hdl].mib_len = sizeof(hdls[hdl].mib) / sizeof(*hdls[hdl].mib);
    if (sysctlnametomib(cfg->ratio, hdls[hdl].mib, &hdls[hdl].mib_len) != 0)
	return PWME_SYSCTLRD;

    /* Mark it as in use */
    hdls[hdl].used = 1;

    /* Initialize the pin to 0% */
    if ((rc = pwm_write(hdl, 0)) < 0)
	return rc;

    return hdl;
}

static inline int
pwm_check_handle(pwm_handle hdl)
{
    if (hdl < 0 || hdl >= MAX_PWM_HANDLES)
	return PWME_BADH;

    if (!hdls[hdl].used)
	return PWME_BADH;

    return PWME_OK;
}

int
pwm_write(pwm_handle hdl, double pct)
{
    int rc = 0;
    if ((rc = pwm_check_handle(hdl)) < 0)
	return rc;

    if (pct < 0 || pct > 1.0)
	return PWME_INVAL;

    uint32_t new_ratio = (uint32_t)((double)hdls[hdl].period * pct);

    if (sysctl(hdls[hdl].mib, hdls[hdl].mib_len,
	       NULL, NULL,
	       &new_ratio, sizeof(new_ratio)) < 0)
	return PWME_SYSCTLWR;

    return PWME_OK;
}

int
pwm_read(pwm_handle hdl, double *pct)
{
    int rc = 0;
    if ((rc = pwm_check_handle(hdl)) < 0)
	return rc;

    if (!pct)
	return PWME_INVAL;

    uint32_t	ratio	   = 0;
    size_t	ratio_size = sizeof(ratio);

    if (sysctl(hdls[hdl].mib, hdls[hdl].mib_len,
	       &ratio, &ratio_size,
	       NULL, 0) < 0)
	return PWME_SYSCTLRD;

    *pct = (double)ratio / (double)hdls[hdl].period;

    return PWME_OK;
}
