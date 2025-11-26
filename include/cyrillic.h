/*
 ! ##########################################
 ! #                                        #
 ! # BEWARE, THIS FILE CONTAINS WAR CRIMES  #
 ! #                                        #
 ! ##########################################
 */


// cyrillic.h
#pragma once

// Cyrillic Aliases

#define распечатать printf
#define очистить_экранраспечатать clear_screen
#define установить_цвет_текста set_text_color
#define квас while
#define вода void
#define водка int
#define иф if
#define русский return
#define бесконечный_цикл while (1)

// English Aliases

#define printa printf
#define intka int
#define voida void
#define charka char
#define ifk if
#define returnk return
#define whileka while

// C Warcrimes

#define ja ((void(*)())0)
#define pierdole ();
#define сука ((void(*)())0)
#define блядь ();
#define fuck ((void(*)())0)
#define you ()
#define compiler ;
#define nique ((void(*)())0)
#define ta ()
#define mère ;
#define yes 0xFFFFFFFFu
#define nuke void *ptr = (void*)0; volatile size_t sz = yes; memset(ptr, 0, sz);

// debug functions, why in this file? because I use it EVERYWHERE

#define GLOBAL_DEBUG 1

#if GLOBAL_DEBUG
    #define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) ((void)0)
#endif