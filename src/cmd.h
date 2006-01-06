/*
 * cmd.h -- part of the colex package
 *
 * Copyright (c) 2006 Pouya D. Tafti
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * author: Pouya D. Tafti
 *
 * $ 2006-01-01 $
 *
 */

/* command keys */
typedef enum {
    Kr8g8b8a8 = 'r'+'8'+'g'+'8'+'b'+'8'+'a'+'8',
    Kred      = 'r'+'e'+'d',
    Kgrn      = 'g'+'r'+'n',
    Kblu      = 'b'+'l'+'u',
    Kwht      = 'w'+'h'+'t',
    KgammaR   = 'g'+'a'+'m'+'m'+'a'+'R',
    KgammaG   = 'g'+'a'+'m'+'m'+'a'+'G',
    KgammaB   = 'g'+'a'+'m'+'m'+'a'+'B',
    Kgamma    = 'g'+'a'+'m'+'m'+'a',
    Kmouse    = 'm'+'o'+'u'+'s'+'e',  
    Kexit     = 'e'+'x'+'i'+'t'
} Cmd;
