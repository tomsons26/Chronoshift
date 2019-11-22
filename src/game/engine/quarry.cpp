/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Holds quarry type enum and conversion functions to/from strings.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "quarry.h"
#include "gamedebug.h"

const char *g_QuarryName[QUARRY_COUNT] = { "N/A",
    "Anything",
    "Buildings - any",
    "Harvesters",
    "Infantry",
    "Vehicles - any",
    "Ships - any",
    "Factories",
    "Base Defenses",
    "Base Threats",
    "Power Facilities",
    "Fake Buildings" };

QuarryType Quarry_From_Name(char const *name)
{
    DEBUG_ASSERT(name != nullptr);

    if (strcasecmp(name, "<none>") == 0 || strcasecmp(name, "none") == 0) {
        return QUARRY_NONE;
    }

    if (name != nullptr) {
        for (QuarryType quarry = QUARRY_FIRST; quarry < QUARRY_COUNT; ++quarry) {
            if (stricmp(name, g_QuarryName[quarry]) == 0) {
                return quarry;
            }
        }
    }

    return QUARRY_NONE;
}

char const *Name_From_Quarry(QuarryType quarry)
{
    DEBUG_ASSERT(quarry != QUARRY_NONE);
    DEBUG_ASSERT(quarry < QUARRY_COUNT);

    if (quarry != QUARRY_NONE && quarry < QUARRY_COUNT) {
        return g_QuarryName[quarry];
    }

    return "None";
}
