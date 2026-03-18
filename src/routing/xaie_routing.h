/******************************************************************************
* Copyright (C) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_routing.h
* @{
*
* This file contains the high level routing APIs for AI-Engines.
*
******************************************************************************/
#ifndef XAIE_ROUTING_H
#define XAIE_ROUTING_H

/***************************** Include Files *********************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/

/* Stream Direction Support Structure */
typedef struct {
    u8 canMoveNorth : 1;
    u8 canMoveSouth : 1;
    u8 canMoveEast : 1;
    u8 canMoveWest : 1;
} XAie_StreamDirSupported;

/* Channel Port Mapping Structure */
typedef struct XAie_ChannelPortMapping {
    u8 channel; /* available channels */
    u8 port; /* available ports */
    bool availability; /* is this channel used? */
} XAie_ChannelPortMapping;

/* Routing Resource Constraint */
typedef struct XAie_RoutingResourceConstraint {
    u8 column; /* specific column */
    XAie_ChannelPortMapping* channelPortMappings;
    u8 channelPortMappingCount;
    bool HostToAie; /* Host-to-aie or aie-to-host */
} XAie_RoutingResourceConstraint;

/* Host AIE Constraint */
typedef struct XAie_HostAieConstraint {
    XAie_RoutingResourceConstraint* RoutingResourceConstraint;
    u8 ConstraintCount;
} XAie_HostAieConstraint;

/* Routing Step Structure */
typedef struct XAie_RoutingStep {
    XAie_LocType sourceTile;
    int sourceStream;
    int destStream;
    StrmSwPortType source_direction;
    StrmSwPortType dest_direction;
    struct XAie_RoutingStep *next; /* Pointer to the next step in the path */
} XAie_RoutingStep;

/* Routing Path Structure */
typedef struct {
    XAie_LocType source;
    XAie_LocType destination;
    u8 MM2S_portNo;
    u8 S2MM_portNo;
    XAie_RoutingStep *nextStep; /* Pointer to the first step in the path */
} XAie_RoutingPath;

/* Programmed Routes Structure */
typedef struct XAie_ProgrammedRoutes {
    XAie_RoutingPath* routePath;
    struct XAie_ProgrammedRoutes* nextRoute;
} XAie_ProgrammedRoutes;

/* S2MM Channels In Use */
typedef struct {
    u8* S2MM_ports;
    u8 S2MM_portCount;
} XAie_s2mmChannelsInUse;

/* MM2S Channels In Use */
typedef struct {
    u8* MM2S_ports;
    u8 MM2S_portCount;
} XAie_mm2sChannelsInUse;

/* Tile Type Enumeration */
typedef enum { XAIE_AIE_SHIM = 0, XAIE_AIE_MEM, XAIE_AIE_CORE } TileType;

/* Core Constraint Structure */
typedef struct {
    bool isAutoConfigured; /* this tile is part of route auto configured by XAie_Route */
    u8 MM2S_State; /* each bit represents a channel state */
    u8 S2MM_State; /* each bit represents a channel state */
    u8 ShimMM2S_State;
    u8 ShimS2MM_State;
    /* DMA is composed of 4 independent channels, two MM2S and two S2MM for AIE tile, 6-S2MM and 6-MM2S for AIE-Mem tile */
    bool AllChannelsInUse;
    u64 BDState; /* each bit represents a BD state [totally 16 BDs available for aie-tile/ 48 for MemTile] */
    bool AllBDsareInUse;
    TileType tile_type;
    XAie_StreamDirSupported DirSupported;
    u8 SlaveEast;
    u8 SlaveWest;
    u8 SlaveSouth;
    u8 SlaveNorth;
    u8 MasterEast;
    u8 MasterWest;
    u8 MasterSouth;
    u8 MasterNorth;
    XAie_s2mmChannelsInUse s2mmChannelsInUse;
    XAie_mm2sChannelsInUse mm2sChannelsInUse;
    XAie_ChannelPortMapping* Host2AIEPorts; /* SHIM Tile's ports that are available for use to move data from Host to AIE Tile */
    u8 Host2AIEPortCount;
    XAie_ChannelPortMapping* AIE2HostPorts; /* SHIM Tile's ports that are available for use to move data from Host to AIE Tile */
    u8 AIE2HostPortCount;
    XAie_ProgrammedRoutes *routesDB; /* Pointer to the routing path data structure */
    /* More constraints can be added per core */
    bool isCoreExecuting; /* if this is true, this means this core */
} XAie_CoreConstraint;

/* Route Constraints Structure */
typedef struct {
    XAie_LocType* BlackListedCores;
    u16 NoOfBlackListedCores;
    XAie_LocType* WhiteListedCores;
    u16 NoOfWhiteListedCores;
} XAie_RouteConstraints;

/* Routing Instance Structure */
typedef struct XAie_RoutingInstance {
    XAie_DevInst* DeviceInstance;
    XAie_CoreConstraint*** CoreConstraintPerCore; /* each core's constraint [col][row] */
    u8 NumRows;
    u8 NumCols;
} XAie_RoutingInstance;

/************************** Function Prototypes  *****************************/
XAIE_AIG_EXPORT XAie_RoutingInstance* XAie_InitRoutingHandler(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_Route(XAie_RoutingInstance *routingInstance, XAie_RouteConstraints* RouteConstraints, 
        XAie_LocType source, XAie_LocType destination);
XAIE_AIG_EXPORT AieRC XAie_MoveData(XAie_RoutingInstance *routingInstance, XAie_LocType source, void* SourceObject, 
        u32 data_size, void* DestinationObject, XAie_LocType destination);
XAIE_AIG_EXPORT AieRC XAie_Run(XAie_RoutingInstance* routingInstance, uint32_t count);

#endif /* XAIE_ROUTING_H */

/** @} */
