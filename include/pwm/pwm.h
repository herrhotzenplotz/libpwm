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

#ifndef MRC_PWM_PWM_H
#define MRC_PWM_PWM_H

#ifndef __FreeBSD__
#error Your operating system is unsupported.
#endif


/* Error codes */
#define PWME_OK          0 /* No error                        */
#define PWME_NOTPWMPIN  -1 /* The given pin is not a pwm pin  */
#define PWME_BUSY       -2 /* The given pin is already in use */
#define PWME_SYSCTLWR   -3 /* Sysctl write failure            */
#define PWME_SYSCTLRD   -4 /* Sysctl read fail                */
#define PWME_BADH       -5 /* Bad pwm handle                  */
#define PWME_INVAL      -6 /* Invalid argument                */

typedef int pwm_handle;

int		 pwm_init(void);
pwm_handle	 pwm_open(int pin);
int		 pwm_write(pwm_handle, double pct);
int		 pwm_read(pwm_handle, double *pct);
const char	*pwm_strerror(int code);

#endif /* MRC_PWM_PWM_H */
