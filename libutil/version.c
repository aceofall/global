/*
 * Copyright (c) 1996, 1997, 1998, 1999
 *             Shigio Yamaguchi. All rights reserved.
 * Copyright (c) 1999, 2000
 *             Tama Communications Corporation. All rights reserved.
 *
 * This file is part of GNU GLOBAL.
 *
 * GNU GLOBAL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNU GLOBAL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>

#include "version.h"

extern int vflag;

const char *copy = "\
Copyright (c) 1996, 1997, 1998, 1999 Shigio Yamaguchi.\n\
Copyright (c) 1999, 2000 Tama Communications Corporation.\n\
GNU GLOBAL comes with NO WARRANTY,\n\
to the extent permitted by law.\n\
You may redistribute copies of GNU GLOBAL\n\
under the terms of the GNU General Public License.\n\
For more information about these matters,\n\
see the files named COPYING.\n\
";
/*
 * version: print version information.
 */
void
version(name, verbose)
	const char *name;
	const int verbose;
{
	if (name == NULL)
		name = progname;
	if (verbose) {
		fprintf(stdout, "%s - GNU GLOBAL %s\n", name, VERSION);
		fprintf(stdout, "%s", copy);
	} else {
		fprintf(stdout, "%s\n", VERSION);
	}
	exit(0);
}