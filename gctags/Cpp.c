/*
 * Copyright (c) 1998, 1999 Shigio Yamaguchi. All rights reserved.
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
 *	This product includes software developed by Shigio Yamaguchi.
 * 4. Neither the name of the author nor the names of its contributors
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
 *	Cpp.c					18-Mar-99
 */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Cpp.h"
#include "gctags.h"
#include "defined.h"
#include "die.h"
#include "locatestring.h"
#include "strbuf.h"
#include "token.h"

static	int	function_definition __P((int));
static	void	condition_macro __P((int));
static	int	reserved __P((char *));

/*
 * #ifdef stack.
 */
static struct {
	short start;		/* level when #if block started */
	short end;		/* level when #if block end */
	short if0only;		/* #if 0 or notdef only */
} pifstack[MAXPIFSTACK], *cur;
static int piflevel;		/* condition macro level */
static int level;		/* brace level */

/*
 * Cpp: read C++ file and pickup tag entries.
 */
void
Cpp()
{
	int	c, cc;
	int	savelevel;
	int	target;
	int	startclass, startthrow, startmacro, startsharp, startequal;
	int	destruct;
	char    classname[MAXTOKEN];
	char    completename[MAXCOMPLETENAME];
	int     classlevel;
	struct {
		char *classname;
		char *terminate;
		int   level;
	} stack[MAXCLASSSTACK];
	const	char *interested = "{}=;~";
	STRBUF	*sb = stropen();

	stack[0].terminate = completename;
	stack[0].level = 0;
	level = classlevel = piflevel = 0;
	savelevel = -1;
	target = (sflag) ? SYM : (rflag) ? REF : DEF;
	startclass = startthrow = startmacro = startsharp = startequal = 0;
	cmode = 1;			/* allow token like '#xxx' */
	crflag = 1;			/* require '\n' as a token */
	cppmode = 1;			/* treat '::' as a reserved word */

	destruct = 0;			/* to pick up destructer */

	while ((cc = nexttoken(interested, reserved)) != EOF) {
		if (cc == '~' && level == stack[classlevel].level) {
			destruct = 1;
			continue;
		}

		switch (cc) {
		case SYMBOL:		/* symbol	*/
			if (startclass || startthrow) {
				if (target == REF && defined(token))
					PUT(token, lineno, sp);
			} else if (peekc(0) == '('/* ) */) {
				if (isnotfunction(token)) {
					if (target == REF && defined(token))
						PUT(token, lineno, sp);
				} else if (level > stack[classlevel].level || startequal || startmacro) {
					if (target == REF && defined(token))
						PUT(token, lineno, sp);
				} else if (level == stack[classlevel].level && !startmacro && !startsharp && !startequal) {
					char	savetok[MAXTOKEN], *saveline;
					int	savelineno = lineno;

					strcpy(savetok, token);
					strstart(sb);
					strnputs(sb, sp, strlen(sp) + 1);
					saveline = strvalue(sb);
					if (function_definition(target))
						/* ignore constructor */
						if (target == DEF && strcmp(stack[classlevel].classname, savetok))
							PUT(savetok, savelineno, saveline);
				}
			} else {
				if (target == SYM)
					PUT(token, lineno, sp);
			}
			break;
		case CPP_CLASS:
			DBG_PRINT(level, "class");
			if ((c = nexttoken(interested, reserved)) == SYMBOL) {
				strcpy(classname, token);
				startclass = 1;
				if (target == DEF)
					PUT(token, lineno, sp);
			}
			break;
		case '{':  /* } */
			DBG_PRINT(level, "{"); /* } */
			++level;
			if (bflag && atfirst) {
				if (wflag && level != 1)
					fprintf(stderr, "Warning: forced level 1 block start by '{' at column 0 [+%d %s].\n", lineno, curfile); /* } */
				level = 1;
			}
			if (startclass) {
				char *p = stack[classlevel].terminate;
				char *q = classname;

				if (++classlevel >= MAXCLASSSTACK)
					die1("class stack over flow.[%s]", curfile);
				if (classlevel > 1)
					*p++ = '.';
				stack[classlevel].classname = p;
				while (*q)
					*p++ = *q++;
				stack[classlevel].terminate = p;
				stack[classlevel].level = level;
				*p++ = 0;
			}
			startclass = startthrow = 0;
			break;
			/* { */
		case '}':
			if (--level < 0) {
				if (wflag)
					fprintf(stderr, "Warning: missing left '{' [+%d %s].\n", lineno, curfile); /* } */
				level = 0;
			}
			if (eflag && atfirst) {
				if (wflag && level != 0) /* { */
					fprintf(stderr, "Warning: forced level 0 block end by '}' at column 0 [+%d %s].\n", lineno, curfile);
				level = 0;
			}
			if (level < stack[classlevel].level)
				*(stack[--classlevel].terminate) = 0;
			/* { */
			DBG_PRINT(level, "}");
			break;
		case '=':
			startequal = 1;
			break;
		case ';':
			startthrow = startequal = 0;
			break;
		case '\n':
			if (startmacro && level != savelevel) {
				if (wflag)
					fprintf(stderr, "Warning: different level before and after #define macro. reseted. [+%d %s].\n", lineno, curfile);
				level = savelevel;
			}
			startmacro = startsharp = 0;
			break;
		/*
		 * #xxx
		 */
		case CP_DEFINE:
			startmacro = 1;
			savelevel = level;
			if ((c = nexttoken(interested, reserved)) != SYMBOL) {
				pushbacktoken();
				break;
			}
			if (peekc(1) == '('/* ) */) {
				if (target == DEF)
					PUT(token, lineno, sp);
				while ((c = nexttoken("()", reserved)) != EOF && c != '\n' && c != /* ( */ ')')
					if (c == SYMBOL && target == SYM)
						PUT(token, lineno, sp);
				if (c == '\n')
					pushbacktoken();
			}
			break;
		case CP_INCLUDE:
		case CP_ERROR:
		case CP_LINE:
		case CP_PRAGMA:
			while ((c = nexttoken(interested, reserved)) != EOF && c != '\n')
				;
			break;
		case CP_IFDEF:
		case CP_IFNDEF:
		case CP_IF:
		case CP_ELIF:
		case CP_ELSE:
		case CP_ENDIF:
		case CP_UNDEF:
			condition_macro(cc);
			while ((c = nexttoken(interested, reserved)) != EOF && c != '\n') {
				if (!((cc == CP_IF || cc == CP_ELIF) && !strcmp(token, "defined")))
					continue;
				if (c == SYMBOL && target == SYM)
					PUT(token, lineno, sp);
			}
			break;
		case CP_SHARP:		/* ## */
			(void)nexttoken(interested, reserved);
			break;
		case CPP_NEW:
			if ((c = nexttoken(interested, reserved)) == SYMBOL)
				if (target == REF && defined(token))
					PUT(token, lineno, sp);
			break;
		case CPP_STRUCT:
			c = nexttoken(interested, reserved);
			if (c == '{' /* } */) {
				pushbacktoken();
				break;
			}
			if (c == SYMBOL)
				if (target == SYM)
					PUT(token, lineno, sp);
			break;
		case CPP_EXTERN:
			if (startmacro)
				break;
			while ((c = nexttoken(interested, reserved)) != EOF && c != ';') {
				switch (c) {
				case CP_IFDEF:
				case CP_IFNDEF:
				case CP_IF:
				case CP_ELIF:
				case CP_ELSE:
				case CP_ENDIF:
				case CP_UNDEF:
					condition_macro(c);
					continue;
				}
				if (startmacro && c == '\n')
					break;
				if (c == '{')
					level++;
				else if (c == '}')
					level--;
				else if (c == SYMBOL)
					if (target == SYM)
						PUT(token, lineno, sp);
			}
			break;
		case CPP_TEMPLATE:
			{
				int level = 0;

				while ((c = nexttoken("<>", reserved)) != EOF) {
					if (c == '<')
						++level;
					else if (c == '>') {
						if (--level == 0)
							break;
					} else if (c == SYMBOL) {
						if (target == SYM)
							PUT(token, lineno, sp);
					}
				}
				if (c == EOF && wflag)
					fprintf(stderr, "Warning: templete <...> isn't closed. [+%d %s].\n", lineno, curfile);
			}
			break;
		case CPP_OPERATOR:
			while ((c = nexttoken(";{", reserved)) != EOF) {
				if (c == '{') {
					pushbacktoken();
					break;
				} else if (c == ';') {
					break;
				} else if (c == SYMBOL) {
					if (target == SYM)
						PUT(token, lineno, sp);
				}
			}
			if (c == EOF && wflag)
				fprintf(stderr, "Warning: '{' doesn't exist after 'operator'. [+%d %s].\n", lineno, curfile); /* } */
			break;
		/* control statement check */
		case CPP_THROW:
			startthrow = 1;
		case CPP_BREAK:
		case CPP_CASE:
		case CPP_CATCH:
		case CPP_CONTINUE:
		case CPP_DEFAULT:
		case CPP_DELETE:
		case CPP_DO:
		case CPP_ELSE:
		case CPP_FOR:
		case CPP_GOTO:
		case CPP_IF:
		case CPP_RETURN:
		case CPP_SWITCH:
		case CPP_TRY:
		case CPP_WHILE:
			if (wflag && !startmacro && level == 0)
				fprintf(stderr, "Warning: Out of function. %8s [+%d %s]\n", token, lineno, curfile);
			break;
		default:
		}
		destruct = 0;
	}
	strclose(sb);
	if (wflag) {
		if (level != 0)
			fprintf(stderr, "Warning: {} block unmatched. (last at level %d.)[+%d %s]\n", level, lineno, curfile);
		if (piflevel != 0)
			fprintf(stderr, "Warning: #if block unmatched. (last at level %d.)[+%d %s]\n", piflevel, lineno, curfile);
	}
}
/*
 * function_definition: return if function definition or not.
 *
 *	r)	target type
 */
static int
function_definition(target)
int	target;
{
	int	c;
	int     brace_level, isdefine;

	brace_level = isdefine = 0;
	while ((c = nexttoken("(,)", reserved)) != EOF) {
		switch (c) {
		case CP_IFDEF:
		case CP_IFNDEF:
		case CP_IF:
		case CP_ELIF:
		case CP_ELSE:
		case CP_ENDIF:
		case CP_UNDEF:
			condition_macro(c);
			continue;
		}
		if (c == '('/* ) */)
			brace_level++;
		else if (c == /* ( */')') {
			if (--brace_level == 0)
				break;
		} else if (c == SYMBOL) {
			if (target == SYM)
				PUT(token, lineno, sp);
		}
	}
	if (c == EOF)
		return 0;
	if (peekc(0) == ';') {
		(void)nexttoken(";", NULL);
		return 0;
	}
	while ((c = nexttoken(",;{}=", reserved)) != EOF) {
		switch (c) {
		case CP_IFDEF:
		case CP_IFNDEF:
		case CP_IF:
		case CP_ELIF:
		case CP_ELSE:
		case CP_ENDIF:
		case CP_UNDEF:
			condition_macro(c);
			continue;
		}
		if (c == SYMBOL || IS_RESERVED(c))
			isdefine = 1;
		else if (c == ';' || c == ',') {
			if (!isdefine)
				break;
		} else if (c == '{' /* } */) {
			pushbacktoken();
			return 1;
		} else if (c == /* { */'}') {
			break;
		} else if (c == '=')
			break;
	}
	return 0;
}

/*
 * condition_macro: 
 *
 *	i)	cc	token
 */
static void
condition_macro(cc)
	int	cc;
{
	cur = &pifstack[piflevel];
	if (cc == CP_IFDEF || cc == CP_IFNDEF || cc == CP_IF) {
		DBG_PRINT(piflevel, "#if");
		if (++piflevel >= MAXPIFSTACK)
			die1("#if pifstack over flow. [%s]", curfile);
		++cur;
		cur->start = level;
		cur->end = -1;
		cur->if0only = 0;
		if (peekc(0) == '0')
			cur->if0only = 1;
		else if ((cc = nexttoken(NULL, reserved)) == SYMBOL && !strcmp(token, "notdef"))
			cur->if0only = 1;
		else
			pushbacktoken();
	} else if (cc == CP_ELIF || cc == CP_ELSE) {
		DBG_PRINT(piflevel - 1, "#else");
		if (cur->end == -1)
			cur->end = level;
		else if (cur->end != level && wflag)
			fprintf(stderr, "Warning: uneven level. [+%d %s]\n", lineno, curfile);
		level = cur->start;
		cur->if0only = 0;
	} else if (cc == CP_ENDIF) {
		if (cur->if0only)
			level = cur->start;
		else if (cur->end != -1) {
			if (cur->end != level && wflag)
				fprintf(stderr, "Warning: uneven level. [+%d %s]\n", lineno, curfile);
			level = cur->end;
		}
		--piflevel;
		DBG_PRINT(piflevel, "#endif");
	}
}
		/* sorted by alphabet */
static struct words words[] = {
	{"##",		CP_SHARP},
	{"#define",	CP_DEFINE},
	{"#elif",	CP_ELIF},
	{"#else",	CP_ELSE},
	{"#endif",	CP_ENDIF},
	{"#error",	CP_ERROR},
	{"#if",		CP_IF},
	{"#ifdef",	CP_IFDEF},
	{"#ifndef",	CP_IFNDEF},
	{"#include",	CP_INCLUDE},
	{"#line",	CP_LINE},
	{"#pragma",	CP_PRAGMA},
	{"#undef",	CP_UNDEF},
	{"::",		CPP_SEP},
	{"__P",		CPP___P},
	{"asm",		CPP_ASM},
	{"auto",	CPP_AUTO},
	{"break",	CPP_BREAK},
	{"case",	CPP_CASE},
	{"catch",	CPP_CATCH},
	{"char",	CPP_CHAR},
	{"class",	CPP_CLASS},
	{"const",	CPP_CONST},
	{"continue",	CPP_CONTINUE},
	{"default",	CPP_DEFAULT},
	{"delete",	CPP_DELETE},
	{"do",		CPP_DO},
	{"double",	CPP_DOUBLE},
	{"else",	CPP_ELSE},
	{"enum",	CPP_ENUM},
	{"extern",	CPP_EXTERN},
	{"float",	CPP_FLOAT},
	{"for",		CPP_FOR},
	{"friend",	CPP_FRIEND},
	{"goto",	CPP_GOTO},
	{"if",		CPP_IF},
	{"inline",	CPP_INLINE},
	{"int",		CPP_INT},
	{"long",	CPP_LONG},
	{"new",		CPP_NEW},
	{"operator",	CPP_OPERATOR},
	{"private",	CPP_PRIVATE},
	{"protected",	CPP_PROTECTED},
	{"public",	CPP_PUBLIC},
	{"register",	CPP_REGISTER},
	{"return",	CPP_RETURN},
	{"short",	CPP_SHORT},
	{"signed",	CPP_SIGNED},
	{"sizeof",	CPP_SIZEOF},
	{"static",	CPP_STATIC},
	{"struct",	CPP_STRUCT},
	{"switch",	CPP_SWITCH},
	{"template",	CPP_TEMPLATE},
	{"this",	CPP_THIS},
	{"throw",	CPP_THROW},
	{"try",		CPP_TRY},
	{"typedef",	CPP_TYPEDEF},
	{"union",	CPP_UNION},
	{"unsigned",	CPP_UNSIGNED},
	{"virtual",	CPP_VIRTUAL},
	{"void",	CPP_VOID},
	{"volatile",	CPP_VOLATILE},
	{"while",	CPP_WHILE},
};

static int
reserved(word)
        char *word;
{
	struct words tmp;
	struct words *result;

	tmp.name = word;
	result = (struct words *)bsearch(&tmp, words, sizeof(words)/sizeof(struct words), sizeof(struct words), cmp);
	return (result != NULL) ? result->val : SYMBOL;
}
/*
 * whether or not C++.
 */
int
isCpp()
{
	int cc;
	int Cpp = 0;
	cmode = 1;			/* allow token like '#xxx' */
	cppmode = 1;			/* treat '::' as a reserved word */

	while ((cc = nexttoken(NULL, reserved)) != EOF) {
		if (cc == CPP_CLASS || cc == CPP_TEMPLATE ||
			cc == CPP_OPERATOR || cc == CPP_VIRTUAL) {
			Cpp = 1;
			break;
		}
	}
	rewindtoken();
	return Cpp;
}