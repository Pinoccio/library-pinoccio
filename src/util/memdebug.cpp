/*
 * memdebug.c
 *
 *  Created on: 15 Dec 2010
 *      Author: Andy Brown
 *
 *  Copyright (c) 2011,2012 Andrew Brown. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *  Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 *  The name of Andrew Brown may not be used to endorse or promote
 *  products derived from this software without specific prior written
 *  permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ANDREW
 *  BROWN BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#include <Arduino.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include "memdebug.h"
#include <src/bitlash.h>

/**
 * This must match the definition in "stdlib_private.h"
 */

typedef struct __freelist {
  size_t sz;
  struct __freelist *nx;
} FREELIST;

extern FREELIST *__flp;
extern char *__brkval;


/**
 * Get the total memory used by your program. The total will
 * include accounting overhead internal to the library
 */

size_t getMemoryUsed()
{
  size_t used;
  FREELIST *fp;

// __brkval=0 if nothing has been allocated yet

  if(__brkval==0)
    return 0;

// __brkval moves up from __malloc_heap_start to
// __malloc_heap_end as memory is used

  used=__brkval-__malloc_heap_start;

// memory free'd by you is collected in the free list and
// compacted with adjacent blocks. This, combined with malloc's
// intelligent picking of candidate blocks drastically reduces
// heap fragmentation. Anyway, since blocks in the free list
// are available to you at no cost we need to take them off.

  for(fp=__flp;fp;fp=fp->nx)
    used-=fp->sz+sizeof(size_t);

  return used;
}


/**
 * Get the total free bytes
 */

size_t getFreeMemory()
{
  return (size_t)AVR_STACK_POINTER_REG-
         (size_t)__malloc_margin-
         (size_t)__malloc_heap_start-
         getMemoryUsed();
}


/**
 * Get the largest available block that can be successfully
 * allocated by malloc()
 */

size_t getLargestAvailableMemoryBlock()
{
  size_t a,b;

  a=getLargestBlockInFreeList();
  b=getLargestNonFreeListBlock();

  return a>b ? a : b;
}


/**
 * Get the largest block in the free list
 */

size_t getLargestBlockInFreeList()
{
  FREELIST *fp;
  size_t maxsize=0;

  for(fp=__flp;fp;fp=fp->nx)
    if(fp->sz>maxsize)
      maxsize=fp->sz;

  return maxsize;
}


/**
 * Get the number of blocks in the free list
 */

int getNumberOfBlocksInFreeList()
{
  FREELIST *fp;
  int i;

  for(i=0,fp=__flp;fp;fp=fp->nx,i++);
  return i;
}


/**
 * Get total size of free list (includes library overhead)
 */

size_t getFreeListSize()
{
  FREELIST *fp;
  size_t size;

  for(size=0,fp=__flp;fp;fp=fp->nx,size+=fp->sz+sizeof(size_t));
  return size;
}


/**
 * Get the largest block that can be successfully allocated
 * without reuse from the free list
 */

size_t getLargestNonFreeListBlock()
{
  char *cp,*brkval;

// this code is an adapted fragment from malloc() itself

  brkval=__brkval == 0 ? __malloc_heap_start : __brkval;

  if((cp=__malloc_heap_end)==NULL)
    cp=(char *)AVR_STACK_POINTER_REG-__malloc_margin;
  if(cp<=brkval)
    return 0;

  return cp-brkval;
}

int showMemory(void) {
  char buffer[100];
  int usedMem = getMemoryUsed();
  int freeMem = getFreeMemory();
  int largeMem = getLargestAvailableMemoryBlock();

  snprintf(buffer, sizeof(buffer), "%04u %04u %04u : used/free/large",
      usedMem,
      freeMem,
      largeMem
    );

  speol(buffer);
  return freeMem;
}
