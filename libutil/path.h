/*
 * Copyright (c) 1996, 1997, 1998, 1999
 *            Shigio Yamaguchi. All rights reserved.
 * Copyright (c) 1999
 *            Tama Communications Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Tama Communications
 *      Corporation and its contributors.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	path.h					26-Aug-99
 *
 */

#ifndef _PATH_H_
#define _PATH_H_

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <unistd.h>
#endif

/*
 * PATHSEP - Define OS-specific directory and path seperators
 * isabspath(p) - Returns true when the given path is absolute
 */
#if defined(_WIN32) && !defined(__CYGWIN__)
#define PATHSEP ";"
#else
#define PATHSEP ":"
#endif

#define isdrivechar(x) (((x) >= 'A' && (x) <= 'Z') || ((x) >= 'a' && (x) <= 'z'))

int	isabspath(char *);
char	*canonpath(char *);

#endif /* ! _PATH_H_ */