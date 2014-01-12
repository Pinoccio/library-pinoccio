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

#ifdef NWK_ENABLE_MULTICAST

/*- Definitions ------------------------------------------------------------*/
#define NWK_GROUP_FREE      0xffff

/*- Prototypes -------------------------------------------------------------*/
static bool nwkGroupSwitch(uint16_t from, uint16_t to);

/*- Variables --------------------------------------------------------------*/
static uint16_t nwkGroups[NWK_GROUPS_AMOUNT];

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
  @brief Initializes the Group module
*****************************************************************************/
void nwkGroupInit(void)
{
  uint8_t i = 0;
  for (i = 0; i < NWK_GROUPS_AMOUNT; i++)
    nwkGroups[i] = NWK_GROUP_FREE;
}

/*************************************************************************//**
  @brief Adds node to the @a group
  @param[in] group Group ID
  @return @c true in case of success and @c false otherwise
*****************************************************************************/
bool NWK_GroupAdd(uint16_t group)
{
  return nwkGroupSwitch(NWK_GROUP_FREE, group);
}

/*************************************************************************//**
  @brief Removes node from the @a group
  @param[in] group Group ID
  @return @c true in case of success and @c false otherwise
*****************************************************************************/
bool NWK_GroupRemove(uint16_t group)
{
  return nwkGroupSwitch(group, NWK_GROUP_FREE);
}

/*************************************************************************//**
  @brief Verifies if node is a member of the @a group
  @param[in] group Group ID
  @return @c true if node is a member of the group and @c false otherwise
*****************************************************************************/
bool NWK_GroupIsMember(uint16_t group)
{
  uint8_t i = 0;
  for (i = 0; i < NWK_GROUPS_AMOUNT; i++)
    if (group == nwkGroups[i])
      return true;
  return false;
}

/*************************************************************************//**
  @brief Switches records with IDs @a from and @a to in the the group table
  @param[in] from Source group ID
  @param[in] to   Destination group ID
  @return @c true if @a from entry was found and @c false otherwise
*****************************************************************************/
static bool nwkGroupSwitch(uint16_t from, uint16_t to)
{
  uint8_t i = 0;
  for (i = 0; i < NWK_GROUPS_AMOUNT; i++)
  {
    if (from == nwkGroups[i])
    {
      nwkGroups[i] = to;
      return true;
    }
  }
  return false;
}

uint16_t* NWK_GetGroups(void) {
  return nwkGroups;
}
#endif // NWK_ENABLE_MULTICAST
