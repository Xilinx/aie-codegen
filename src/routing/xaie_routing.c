/******************************************************************************
 * Copyright (c) 2024 AMD, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 * @file xaie_routing.c
 * @{
 *
 * This file contains the high level routing APIs for AI-Engines.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- -----------------------------------------------------
 * 1.0   AIE-CG  02/25/2026  Initial creation for AIE-Codegen project
 * </pre>
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include <stdlib.h>
#include "xaie_routing.h"
#include "xaie_helper.h"
#include "xaie_core.h"

/*****************************************************************************/
/**
*
* This API initializes a routing handler instance for the device.
*
* @param	DevInst: Device Instance
*
* @return	XAie_RoutingInstance pointer on success, NULL on failure.
*
* @note		This function allocates memory for the routing instance and
*		initializes it based on the device characteristics.
*
******************************************************************************/
XAie_RoutingInstance* XAie_InitRoutingHandler(XAie_DevInst *DevInst)
{
	(void)DevInst;
	XAIE_ERROR("XAie_InitRoutingHandler not supported in aie-codegen\n");
	return NULL;
}

/*****************************************************************************/
/**
*
* This API creates a route between a source and destination tile. If a route 
* already exists between the two, the API fails.
*
* @param	routingInstance: Routing Instance
* @param	RouteConstraints: Route Constraints (can be NULL)
* @param	source: Source tile location
* @param	destination: Destination tile location
*
* @return	XAIE_OK if successful, error code on failure.
*
* @note		This is a simplified implementation. In a full implementation,
*		this would involve pathfinding algorithms and stream switch
*		configuration.
*
******************************************************************************/
AieRC XAie_Route(XAie_RoutingInstance *routingInstance, XAie_RouteConstraints* RouteConstraints,
		XAie_LocType source, XAie_LocType destination)
{
	(void)routingInstance;
	(void)RouteConstraints;
	(void)source;
	(void)destination;

	XAIE_ERROR("XAie_Route not supported in aie-codegen\n");
	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API moves data between a source and destination tile along a configured route.
*
* @param	routingInstance: Routing Instance
* @param	source: Source tile location
* @param	SourceObject: Source object (memory instance or address)
* @param	data_size: Size of data to move in bytes
* @param	DestinationObject: Destination object (memory instance or address) 
* @param	destination: Destination tile location
*
* @return	XAIE_OK if successful, error code on failure.
*
* @note		This is a simplified implementation. In a full implementation,
*		this would configure DMAs and manage data transfer.
*
******************************************************************************/
AieRC XAie_MoveData(XAie_RoutingInstance *routingInstance, XAie_LocType source, void* SourceObject,
		u32 data_size, void* DestinationObject, XAie_LocType destination)
{
	(void)routingInstance;
	(void)source;
	(void)SourceObject;
	(void)data_size;
	(void)DestinationObject;
	(void)destination;
	XAIE_ERROR("XAie_MoveData not supported in aie-codegen\n");
	return XAIE_ERR;
}
