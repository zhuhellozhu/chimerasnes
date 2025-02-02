#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#ifndef HAVE_NO_LANGEXTRA
	#include "libretro_core_options_intl.h"
#endif

/*
 ********************************
 * VERSION: 2.0
 ********************************
 *
 * - 2.0: Add support for core options v2 interface
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Core Option Definitions */

/* RETRO_LANGUAGE_ENGLISH */

/* Default language:
 * - All other languages must include the same keys and values
 * - Will be used as a fallback in the event that frontend language
 *   is not available
 * - Will be used as a fallback for any missing entries in
 *   frontend language definition */
struct retro_core_option_v2_category option_cats_us[] =
{
	{
		"hacks",
		"Emulation Hacks",
		"Configure processor overclocking and emulation accuracy parameters affecting low-level performance and compatibility."
	},
	{ NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_us[] =
{
	{
		"chimerasnes_frameskip",
		"Frameskip",
		NULL,
		"Skip frames to avoid audio buffer under-run (crackling). Improves performance at the expense of visual smoothness. 'Auto' skips frames when advised by the frontend. 'Manual' utilises the 'Frameskip Threshold (%)' setting.",
		NULL,
		NULL,
		{
			{ "disabled", NULL },
			{ "auto",     "Auto" },
			{ "manual",   "Manual" },
			{ NULL,       NULL },
		},
		"auto"
	},
	{
		"chimerasnes_frameskip_threshold",
		"Frameskip Threshold (%)",
		NULL,
		"When 'Frameskip' is set to 'Manual', specifies the audio buffer occupancy threshold (percentage) below which frames will be skipped. Higher values reduce the risk of crackling by causing frames to be dropped more frequently.",
		NULL,
		NULL,
		{
			{ "15", NULL },
			{ "18", NULL },
			{ "21", NULL },
			{ "24", NULL },
			{ "27", NULL },
			{ "30", NULL },
			{ "33", NULL },
			{ "36", NULL },
			{ "39", NULL },
			{ "42", NULL },
			{ "45", NULL },
			{ "48", NULL },
			{ "51", NULL },
			{ "54", NULL },
			{ "57", NULL },
			{ "60", NULL },
			{ NULL, NULL },
		},
		"33"
	},
	{
		"chimerasnes_overclock_cycles",
		"Reduce Slowdown (Unsafe, Restart)",
		NULL,
		"Overclock SNES CPU. May cause games to crash! Use 'Light' for shorter loading times, 'Compatible' for most games exhibiting slowdown and 'Max' only if absolutely required.",
		NULL,
		"hacks",
		{
			{ "disabled",   NULL },
			{ "light",      "Light" },
			{ "compatible", "Compatible" },
			{ "max",        "Max" },
			{ NULL,         NULL },
		},
		"disabled"
	},
	{
		"chimerasnes_overclock_superfx",
		"SuperFX Overclock",
		NULL,
		"Overclock or underclock the SuperFX chip. This may improve the framerate and playability of games that use SuperFX.",
		NULL,
		"hacks",
		{
			{ "5 MHz (Underclock)",           NULL },
			{ "6 MHz (Underclock)",           NULL },
			{ "7 MHz (Underclock)",           NULL },
			{ "8 MHz (Underclock)",           NULL },
			{ "9 MHz (Underclock)",           NULL },
			{ "10 MHz (SNES clock, Default)", NULL },
			{ "11 MHz (Overclock)",           NULL },
			{ "12 MHz (Overclock)",           NULL },
			{ "13 MHz (Overclock)",           NULL },
			{ "14 MHz (Overclock)",           NULL },
			{ "15 MHz (Overclock)",           NULL },
			{ "20 MHz (Overclock)",           NULL },
			{ "30 MHz (Overclock)",           NULL },
			{ "40 MHz (Overclock)",           NULL },
			{ NULL,                           NULL},
		},
		"10 MHz (SNES clock, Default)"
	},
	{
		"chimerasnes_reduce_sprite_flicker",
		"Reduce Flickering (Unsafe)",
		NULL,
		"Increases number of sprites that can be drawn simultaneously on screen. Reduces flickering in games.",
		NULL,
		"hacks",
		{
			{ "disabled", NULL },
			{ "enabled",  NULL },
			{ NULL,       NULL },
		},
		"disabled"
	},
	{ NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_us =
{
	option_cats_us,
	option_defs_us
};

/* Language Mapping */

#ifndef HAVE_NO_LANGEXTRA
struct retro_core_options_v2 *options_intl[RETRO_LANGUAGE_LAST] =
{
	&options_us, /* RETRO_LANGUAGE_ENGLISH */
	NULL,        /* RETRO_LANGUAGE_JAPANESE */
	NULL,        /* RETRO_LANGUAGE_FRENCH */
	NULL,        /* RETRO_LANGUAGE_SPANISH */
	NULL,        /* RETRO_LANGUAGE_GERMAN */
	NULL,        /* RETRO_LANGUAGE_ITALIAN */
	NULL,        /* RETRO_LANGUAGE_DUTCH */
	NULL,        /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
	NULL,        /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
	NULL,        /* RETRO_LANGUAGE_RUSSIAN */
	NULL,        /* RETRO_LANGUAGE_KOREAN */
	NULL,        /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
	NULL,        /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
	NULL,        /* RETRO_LANGUAGE_ESPERANTO */
	NULL,        /* RETRO_LANGUAGE_POLISH */
	NULL,        /* RETRO_LANGUAGE_VIETNAMESE */
	NULL,        /* RETRO_LANGUAGE_ARABIC */
	NULL,        /* RETRO_LANGUAGE_GREEK */
	NULL,        /* RETRO_LANGUAGE_TURKISH */
	NULL,        /* RETRO_LANGUAGE_SLOVAK */
	NULL,        /* RETRO_LANGUAGE_PERSIAN */
	NULL,        /* RETRO_LANGUAGE_HEBREW */
	NULL,        /* RETRO_LANGUAGE_ASTURIAN */
	NULL,        /* RETRO_LANGUAGE_FINNISH */
};

/* Functions */

/* Handles configuration/setting of core options.
 * Should be called as early as possible - ideally inside
 * retro_set_environment(), and no later than retro_load_game()
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs) */
static INLINE void libretro_set_core_options(retro_environment_t environ_cb, bool *categories_supported)
{
	unsigned version  = 0;

#ifndef HAVE_NO_LANGEXTRA
	unsigned language = 0;
#endif

	if (!environ_cb || !categories_supported)
		return;

	*categories_supported = false;

	if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
		version = 0;

	if (version >= 2)
	{
	#ifndef HAVE_NO_LANGEXTRA
		struct retro_core_options_v2_intl core_options_intl;
		core_options_intl.us    = &options_us;
		core_options_intl.local = NULL;

		if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) && (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
			core_options_intl.local = options_intl[language];

		*categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL, &core_options_intl);
	#else
		*categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2, &options_us);
	#endif
	}
	else
	{
		size_t i, j;
		size_t option_index                                         = 0;
		size_t num_options                                          = 0;
		struct retro_core_option_definition* option_v1_defs_us      = NULL;

	#ifndef HAVE_NO_LANGEXTRA
		size_t num_options_intl                                     = 0;
		struct retro_core_option_v2_definition* option_defs_intl    = NULL;
		struct retro_core_option_definition*    option_v1_defs_intl = NULL;
		struct retro_core_options_intl          core_options_v1_intl;
	#endif

		struct retro_variable *variables = NULL;
		char **values_buf                = NULL;

		while (true) /* Determine total number of options */
		{
			if (option_defs_us[num_options].key)
				num_options++;
			else
				break;
		}

		if (version >= 1)
		{
			/* Allocate US array */
			option_v1_defs_us = (struct retro_core_option_definition *) calloc(num_options + 1, sizeof(struct retro_core_option_definition));

			for (i = 0; i < num_options; i++) /* Copy parameters from option_defs_us array */
			{
				struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
				struct retro_core_option_value *option_values         = option_def_us->values;
				struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
				struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;
				option_v1_def_us->key                                 = option_def_us->key;
				option_v1_def_us->desc                                = option_def_us->desc;
				option_v1_def_us->info                                = option_def_us->info;
				option_v1_def_us->default_value                       = option_def_us->default_value;

				while (option_values->value) /* Values must be copied individually... */
				{
					option_v1_values->value = option_values->value;
					option_v1_values->label = option_values->label;
					option_values++;
					option_v1_values++;
				}
			}

		#ifndef HAVE_NO_LANGEXTRA
			if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) && (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH) && options_intl[language])
				option_defs_intl = options_intl[language]->definitions;

			if (option_defs_intl)
			{
				/* Determine number of intl options */
				while (true)
				{
					if (option_defs_intl[num_options_intl].key)
						num_options_intl++;
					else
						break;
				}

				/* Allocate intl array */
				option_v1_defs_intl = (struct retro_core_option_definition*) calloc(num_options_intl + 1, sizeof(struct retro_core_option_definition));

				for (i = 0; i < num_options_intl; i++) /* Copy parameters from option_defs_intl array */
				{
					struct retro_core_option_v2_definition *option_def_intl = &option_defs_intl[i];
					struct retro_core_option_value *option_values           = option_def_intl->values;
					struct retro_core_option_definition *option_v1_def_intl = &option_v1_defs_intl[i];
					struct retro_core_option_value *option_v1_values        = option_v1_def_intl->values;
					option_v1_def_intl->key                                 = option_def_intl->key;
					option_v1_def_intl->desc                                = option_def_intl->desc;
					option_v1_def_intl->info                                = option_def_intl->info;
					option_v1_def_intl->default_value                       = option_def_intl->default_value;

					while (option_values->value) /* Values must be copied individually... */
					{
						option_v1_values->value = option_values->value;
						option_v1_values->label = option_values->label;
						option_values++;
						option_v1_values++;
					}
				}
			}

			core_options_v1_intl.us    = option_v1_defs_us;
			core_options_v1_intl.local = option_v1_defs_intl;
			environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_v1_intl);
		#else
			environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
		#endif
		}
		else
		{
			/* Allocate arrays */
			variables  = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
			values_buf = (char **)calloc(num_options, sizeof(char *));

			if (!variables || !values_buf)
				goto error;

			for (i = 0; i < num_options; i++) /* Copy parameters from option_defs_us array */
			{
				const char *key                        = option_defs_us[i].key;
				const char *desc                       = option_defs_us[i].desc;
				const char *default_value              = option_defs_us[i].default_value;
				struct retro_core_option_value *values = option_defs_us[i].values;
				size_t buf_len                         = 3;
				size_t default_index                   = 0;
				values_buf[i]                          = NULL;

				if (desc)
				{
					size_t num_values = 0;

					while (true) /* Determine number of values */
					{
						if (values[num_values].value)
						{
							if (default_value) /* Check if this is the default value */
								if (strcmp(values[num_values].value, default_value) == 0)
									default_index = num_values;

							buf_len += strlen(values[num_values].value);
							num_values++;
						}
						else
							break;
					}

					if (num_values > 0) /* Build values string */
					{
						buf_len      += num_values - 1;
						buf_len      += strlen(desc);
						values_buf[i] = (char *)calloc(buf_len, sizeof(char));

						if (!values_buf[i])
							goto error;

						strcpy(values_buf[i], desc);
						strcat(values_buf[i], "; ");

						/* Default value goes first */
						strcat(values_buf[i], values[default_index].value);

						/* Add remaining values */
						for (j = 0; j < num_values; j++)
						{
							if (j != default_index)
							{
								strcat(values_buf[i], "|");
								strcat(values_buf[i], values[j].value);
							}
						}
					}
				}

				variables[option_index].key   = key;
				variables[option_index].value = values_buf[i];
				option_index++;
			}

			/* Set variables */
			environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
		}

	error:
		/* Clean up */

		if (option_v1_defs_us)
		{
			free(option_v1_defs_us);
			option_v1_defs_us = NULL;
		}

#ifndef HAVE_NO_LANGEXTRA
		if (option_v1_defs_intl)
		{
			free(option_v1_defs_intl);
			option_v1_defs_intl = NULL;
		}
#endif

		if (values_buf)
		{
			for (i = 0; i < num_options; i++)
			{
				if (values_buf[i])
				{
					free(values_buf[i]);
					values_buf[i] = NULL;
				}
			}

			free(values_buf);
			values_buf = NULL;
		}

		if (variables)
		{
			free(variables);
			variables = NULL;
		}
	}
}

#ifdef __cplusplus
}
#endif
#endif
#endif
