/*
 * Copyright (c) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005
 *	Tama Communications Corporation
 *
 * This file is part of GNU GLOBAL.
 *
 * GNU GLOBAL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * GNU GLOBAL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include "queue.h"
#include "global.h"
#include "cache.h"
#include "htags.h"
#include "path2url.h"
#include "common.h"

/*
 * makedefineindex: make definition index (including alphabetic index)
 *
 *	i)	file		definition index file
 *	i)	total		definitions total
 *	o)	@defines
 *	gi)	tag cache
 */
int
makedefineindex(file, total, defines)
	const char *file;
	int total;
	STRBUF *defines;
{
	int count = 0;
	int alpha_count = 0;
	FILE *MAP = NULL;
	FILE *DEFINES, *STDOUT, *TAGS, *ALPHA = NULL;
	STRBUF *sb = strbuf_open(0);
	STRBUF *url = strbuf_open(0);
	/* Index link */
	const char *target = (Fflag) ? "mains" : "_top";
	const char *indexlink;
	const char *index_string = "Index Page";
	char command[1024], buf[1024], alpha[32], alpha_f[32], *_;

	if (!aflag && !Fflag)
		indexlink = "mains";
	else if (Fflag)
		indexlink = "../defines";
	else
		indexlink = "../mains";

	if (map_file) {
		if (!(MAP = fopen(makepath(distpath, "MAP", NULL), "w")))
			die("cannot open '%s'.", makepath(distpath, "MAP", NULL));
	}
	if (!(DEFINES = fopen(makepath(distpath, file, NULL), "w")))
		die("cannot make function index '%s'.", file);
	fputs_nl(gen_page_begin(title_define_index, TOPDIR), DEFINES);
	fputs_nl(body_begin, DEFINES);
	fputs(header_begin, DEFINES);
	if (Fflag)
		fputs(gen_href_begin(NULL, "defines", normal_suffix, NULL), DEFINES);
	fputs(title_define_index, DEFINES);
	if (Fflag)
		fputs(gen_href_end(), DEFINES);
	fputs_nl(header_end, DEFINES);
	if (!aflag && !Fflag) {
		fputs(gen_href_begin_with_title(NULL, indexlink, normal_suffix, NULL, index_string), DEFINES);
		if (icon_list)
			fputs(gen_image(CURRENT, back_icon, ".."), DEFINES);
		else
			fputs("[..]", DEFINES);
		fputs_nl(gen_href_end(), DEFINES);
	}
	if (!aflag) {
		if (!no_order_list)
			fputs_nl(list_begin, DEFINES);
	}
	/*
	 * map DEFINES to STDOUT.
	 */
	STDOUT = DEFINES;
	snprintf(command, sizeof(command), "global -c");
	if ((TAGS = popen(command, "r")) == NULL)
		die("cannot fork.");
	alpha[0] = '\0';
	while ((_ = strbuf_fgets(sb, TAGS, STRBUF_NOCRLF)) != NULL) {
		const char *tag, *line;
		char guide[1024], url_for_map[1024];

		count++;
		tag = _;
		message(" [%d/%d] adding %s", count, total, tag);
		if (aflag && (alpha[0] == '\0' || strncmp(tag, alpha, strlen(alpha)))) {
			const char *msg = (alpha_count == 1) ? "definition is contained." : "definitions are contained.";
			int c;

			if (alpha[0]) {
				char tmp[128];
				snprintf(tmp, sizeof(tmp), "%d %s", alpha_count, msg);
				strbuf_puts(defines, gen_href_begin_with_title("defines", alpha_f, HTML, NULL, tmp));
				strbuf_sprintf(defines, "[%s]", alpha);
				strbuf_puts_nl(defines, gen_href_end());
				alpha_count = 0;
				if (!no_order_list)
					fputs_nl(list_end, ALPHA);
				else
					fputs_nl(br, ALPHA);
				fputs(gen_href_begin_with_title(NULL, indexlink, normal_suffix, NULL, index_string), ALPHA);
				if (icon_list)
					fputs(gen_image(PARENT, back_icon, ".."), ALPHA);
				else
					fputs("[..]", ALPHA);
				fputs_nl(gen_href_end(), ALPHA);
				fputs_nl(body_end, ALPHA);
				fputs_nl(gen_page_end(), ALPHA);
				if (cflag) {
					if (pclose(ALPHA) != 0)
						die("terminated abnormally.");
				} else
					fclose(ALPHA);
				file_count++;
			}
			/*
			 * setup index char (for example, 'a' of '[a]').
			 * alpha is used for display.
			 * alpha_f is used for part of path.
			 */
			c = (unsigned char)*tag;
			if (c > 127) {
				int i2 = *(tag + 1) & 0xff;
				/*
				 * for multi-byte(EUC) code.
				 */
				alpha[0] = *tag;
				alpha[1] = *(tag + 1);
				alpha[2] = '\0';
				snprintf(alpha_f, sizeof(alpha_f), "%03d%03d", c, i2);
			} else if (isalpha(c) || c == '_') {
				alpha[0] = *tag;
				alpha[1] = '\0';
				/*
				 * for CD9660 or FAT file system
				 */
				if (islower(c)) {
					alpha_f[0] = 'l';
					alpha_f[1] = *tag;
					alpha_f[2] = '\0';
				} else {
					alpha_f[0] = *tag;
					alpha_f[1] = '\0';
				}
			} else {
				alpha[0] = *tag;
				alpha[1] = '\0';
				snprintf(alpha_f, sizeof(alpha_f), "%03d", c);
			}
			if (cflag) {
				snprintf(buf, sizeof(buf), "gzip -c >%s/defines/%s.%s", distpath, alpha_f, HTML);
				ALPHA = popen(buf, "w");
			} else {
				snprintf(buf, sizeof(buf), "%s/defines/%s.%s", distpath, alpha_f, HTML);
				ALPHA = fopen(buf, "w");
			}
			if (!ALPHA)
				die("cannot make alphabetical function index.");
			snprintf(buf, sizeof(buf), "[%s]", alpha);
			fputs_nl(gen_page_begin(buf, SUBDIR), ALPHA);
			fputs_nl(body_begin, ALPHA);
			fprintf(ALPHA, "%s[%s]%s\n", header_begin, alpha, header_end);
			fputs(gen_href_begin_with_title(NULL, indexlink, normal_suffix, NULL, index_string), ALPHA);
			if (icon_list)
				fputs(gen_image(PARENT, back_icon, ".."), ALPHA);
			else
				fputs("[..]", ALPHA);
			fputs_nl(gen_href_end(), ALPHA);
			if (!no_order_list)
				fputs_nl(list_begin, ALPHA);
			else
				fprintf(ALPHA, "%s%s\n", br, br);
			STDOUT = ALPHA;
		}
		alpha_count++;
		/*
		 * generating url for function definition.
	 	 */
		line = cache_get(GTAGS, tag);
		strbuf_reset(url);

		if (line == NULL)
			die("internal error in makedefineindex()."); 
		if (*line == ' ') {
			SPLIT ptable;
			const char *fid, *enumber;

			if (split((char *)line + 1, 2, &ptable) < 2) {
				recover(&ptable);
				die("too small number of parts in makedefineindex().\n'%s'", line);
			}
			fid     = ptable.part[0].start;
			enumber = ptable.part[1].start;
			snprintf(url_for_map, sizeof(url_for_map), "%s/%s.%s",
				DEFS, fid, HTML);
			/*
			 * cache record: " <file id> <entry number>"
			 */
			if (dynamic) {
				if (*action != '/' && aflag)
					strbuf_puts(url, "../");
				strbuf_puts(url, action);
				strbuf_sprintf(url, "?pattern=%s%stype=definitions", tag, quote_amp);
			} else {
				if (aflag)
					strbuf_puts(url, "../");
				strbuf_sprintf(url, "%s/%s.%s", DEFS, fid, HTML);
			}
			snprintf(guide, sizeof(guide), "Multiple defined in %s places.", enumber);
			recover(&ptable);
		} else {
			SPLIT ptable;
			const char *lno, *fid, *path;

			if (split((char *)line, 3, &ptable) < 3) {
				recover(&ptable);
				die("too small number of parts in makedefineindex().\n'%s'", line);
			}
			lno = ptable.part[0].start;
			path = ptable.part[1].start;
			path += 2;		/* remove './' */

			fid = path2fid(path);
			snprintf(url_for_map, sizeof(url_for_map), "%s/%s.%s#L%s",
				SRCS, fid, HTML, lno);
			if (aflag)
				strbuf_puts(url, "../");
			strbuf_sprintf(url, "%s/%s.%s#L%s", SRCS, fid, HTML, lno);
			snprintf(guide, sizeof(guide), "Defined at %s in %s.", lno, path);
			recover(&ptable);
		}
		if (!no_order_list)
			fputs(item_begin, STDOUT);
		fputs(gen_href_begin_with_title_target(NULL, strbuf_value(url), NULL, NULL, guide, target), STDOUT);
		fputs(tag, STDOUT);
		fputs(gen_href_end(), STDOUT);
		if (!no_order_list)
			fputs(item_end, STDOUT);
		else
			fputs(br, STDOUT);
		fputc('\n', STDOUT);
		if (map_file)
			fprintf(MAP, "%s\t%s\n", tag, url_for_map);
	}
	if (pclose(TAGS) != 0)
		die("'%s' failed.", command);
	if (aflag && alpha[0]) {
		char tmp[128];
		const char *msg = (alpha_count == 1) ? "definition is contained." : "definitions are contained.";

		snprintf(tmp, sizeof(tmp), "%d %s", alpha_count, msg);
		strbuf_puts(defines, gen_href_begin_with_title("defines", alpha_f, HTML, NULL, tmp));
		strbuf_sprintf(defines, "[%s]", alpha);
		strbuf_puts_nl(defines, gen_href_end());
		if (!no_order_list)
			fputs_nl(list_end, ALPHA);
		else
			fputs_nl(br, ALPHA);
		fputs(gen_href_begin_with_title(NULL, indexlink, normal_suffix, NULL, index_string), ALPHA);
		if (icon_list)
			fputs(gen_image(PARENT, back_icon, ".."), ALPHA);
		else
			fputs("[..]", ALPHA);
		fputs_nl(gen_href_end(), ALPHA);
		fputs_nl(body_end, ALPHA);
		fputs_nl(gen_page_end(), ALPHA);
		fclose(ALPHA);
		file_count++;

		fputs(strbuf_value(defines), DEFINES);
	}
	if (!no_order_list && !aflag)
		fputs_nl(list_end, DEFINES);
	if (!aflag && !Fflag) {
		fputs(gen_href_begin_with_title(NULL, "mains", normal_suffix, NULL, index_string), DEFINES);
		if (icon_list)
			fputs(gen_image(CURRENT, back_icon, ".."), DEFINES);
		else
			fputs("[..]", DEFINES);
		fputs_nl(gen_href_end(), DEFINES);
	}
	fputs_nl(body_end, DEFINES);
	fputs_nl(gen_page_end(), DEFINES);
	fclose(DEFINES);
	file_count++;
	if (map_file)
		fclose(MAP);
	strbuf_close(sb);
	strbuf_close(url);
	return count;
}