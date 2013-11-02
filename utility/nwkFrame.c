/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

/*- Includes ---------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sysConfig.h"
#include "nwk.h"
#include "nwkFrame.h"

/*- Types ------------------------------------------------------------------*/
enum
{
  NWK_FRAME_STATE_FREE = 0x00,
};

/*- Variables --------------------------------------------------------------*/
static NwkFrame_t nwkFrameFrames[NWK_BUFFERS_AMOUNT];

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
  @brief Initializes the Frame module
*****************************************************************************/
void nwkFrameInit(void)
{
  uint8_t i = 0;
  for (i=0; i < NWK_BUFFERS_AMOUNT; i++)
    nwkFrameFrames[i].state = NWK_FRAME_STATE_FREE;
}

/*************************************************************************//**
  @brief Allocates an empty frame from the buffer pool
  @return Pointer to the frame or @c NULL if there are no free frames
*****************************************************************************/
NwkFrame_t *nwkFrameAlloc(void)
{
  uint8_t i = 0;
  for (i = 0; i < NWK_BUFFERS_AMOUNT; i++)
  {
    if (NWK_FRAME_STATE_FREE == nwkFrameFrames[i].state)
    {
      memset(&nwkFrameFrames[i], 0, sizeof(NwkFrame_t));
      nwkFrameFrames[i].size = sizeof(NwkFrameHeader_t);
      nwkFrameFrames[i].payload = nwkFrameFrames[i].data + sizeof(NwkFrameHeader_t);
      return &nwkFrameFrames[i];
    }
  }
  return NULL;
}

/*************************************************************************//**
  @brief Frees a @a frame and returns it to the buffer pool
  @param[in] frame Pointer to the frame to be freed
*****************************************************************************/
void nwkFrameFree(NwkFrame_t *frame)
{
  frame->state = NWK_FRAME_STATE_FREE;
}

/*************************************************************************//**
  @brief Cycles through the allocated frames starting from the specified @a frame
  @param[in] frame Pointer to the current frame or @c NULL for the first frame
  @return Next allocated frame or @c NULL if there are no more frames
*****************************************************************************/
NwkFrame_t *nwkFrameNext(NwkFrame_t *frame)
{
  if (NULL == frame) 
      frame = nwkFrameFrames; 
    else 
      frame++; 

    for (; frame < &nwkFrameFrames[NWK_BUFFERS_AMOUNT]; frame++) 
    { 
      if (NWK_FRAME_STATE_FREE != frame->state) 
        return frame; 
    } 

    return NULL;
}

/*************************************************************************//**
  @brief Sets default parameters for the the command @a frame
  @param[in] frame Pointer to the command frame
*****************************************************************************/
void nwkFrameCommandInit(NwkFrame_t *frame)
{
  frame->tx.status = NWK_SUCCESS_STATUS;
  frame->header.nwkSeq = ++nwkIb.nwkSeqNum;
  frame->header.nwkSrcAddr = nwkIb.addr;
#ifdef NWK_ENABLE_SECURE_COMMANDS
  frame->header.nwkFcf.security = 1;
#endif
}
