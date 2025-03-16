#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_DEBUG_SPHERE(Location) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 12, FColor::Red, true) 
#define DRAW_DEBUG_SPHERE_COLOR(Location, Color) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 12, Color, false, 5.f) 
#define DRAW_DEBUG_SPHERE_SingleFrame(Location) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 12, FColor::Red, false, -1.f) 
#define DRAW_DEBUG_LINE(Location, Forward) if(GetWorld()) DrawDebugLine(GetWorld(), Location, Location + Forward * 100.f, FColor::Cyan, true);
#define DRAW_DEBUG_LINE_SingleFrame(Location, Forward) if(GetWorld()) DrawDebugLine(GetWorld(), Location, Location + Forward * 100.f, FColor::Cyan, false, -1.f);
#define DRAW_DEBUG_POINT(Location) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, 10.f, FColor::Cyan, true);
#define DRAW_DEBUG_POINT_SingleFrame(Location) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, 10.f, FColor::Cyan, false, -1.f);