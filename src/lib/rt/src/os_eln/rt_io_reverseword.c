/* 
 * Proview   $Id: rt_io_reverseword.c,v 1.2 2005-09-01 14:57:56 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* rt_io_reverseword.c
   Rutinen io_ReverseWord spegelv�nder ett 16-bitars ord.  */

#include "pwr.h"
#include "rt_io_reverseword.h"

#define	    IO_NOREV	256

static unsigned short	S_ReverseTab[IO_NOREV] = 
			    {	0, 128, 64, 192, 32, 160, 96, 224,

				16, 144, 80, 208, 48, 176, 112, 240,

				8, 136, 72, 200, 40, 168, 104, 232,
				24, 152, 88, 216, 56, 184, 120, 248,

				4, 132, 68, 196, 36, 164, 100, 228,
				20, 148, 84, 212, 52, 180, 116, 244,
				12, 140, 76, 204, 44, 172, 108, 236,
				28, 156, 92, 220, 60, 188, 124, 252,

				2, 130, 66, 194, 34, 162, 98, 226,
				18, 146, 82, 210, 50, 178, 114, 242,
				10, 138, 74, 202, 42, 170, 106, 234,
				26, 154, 90, 218, 58, 186, 122, 250,
				6, 134, 70, 198, 38, 166, 102, 230,
				22, 150, 86, 214, 54, 182, 118, 246,
				14, 142, 78, 206, 46, 174, 110, 238,
				30, 158, 94, 222, 62, 190, 126, 254,

			    	1, 129, 65, 193, 33, 161, 97, 225,

				17, 145, 81, 209, 49, 177, 113, 241,

				9, 137, 73, 201, 41, 169, 105, 233,
				25, 153, 89, 217, 57, 185, 121, 249,

				5, 133, 69, 197, 37, 165, 101, 229,
				21, 149, 85, 213, 53, 181, 117, 245,
				13, 141, 77, 205, 45, 173, 109, 237,
				29, 157, 93, 221, 61, 189, 125, 253,

				3, 131, 67, 195, 35, 163, 99, 227,
				19, 147, 83, 211, 51, 179, 115, 243,
				11, 139, 75, 203, 43, 171, 107, 235,
				27, 155, 91, 219, 59, 187, 123, 251,
				7, 135, 71, 199, 39, 167, 103, 231,
				23, 151, 87, 215, 55, 183, 119, 247,
				15, 143, 79, 207, 47, 175, 111, 239,
				31, 159, 95, 223, 63, 191, 127, 255 };

				    

/************************************************************************
*
* Namn		: io_ReverseWord
*
* Typ		: unsigned short
*
* TYP		PARAMETER	IUGFR	BESKRIVNING
* IO_WORD_UNION	*Word		I	16-bitars ord som ska spegelv�ndas 
*
* Upphov	: Ulla Tranesj�				(Revision, se filhuvud)
*
* Beskrivning	: Spegelv�nder ett 16-bitars ord genom att tolka ordet som tv�
*		  bytes. Dessa �r index i en tabell S_ReverseTab som inneh�ller
*		  det spegelv�nda ordet.
*************************************************************************/

unsigned short io_ReverseWord(Word)
IO_WORD_UNION *Word;
{
    IO_WORD_UNION   	ReturnValue;

    ReturnValue.B[0] = S_ReverseTab[Word->B[1]];
    ReturnValue.B[1] = S_ReverseTab[Word->B[0]];

    return( ReturnValue.W);

} /* END io_reverseword */
