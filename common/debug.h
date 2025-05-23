/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#include "common/scummsys.h"

#ifdef DISABLE_TEXT_CONSOLE

inline void debug(const char *s, ...) {}
inline void debug(int level, const char *s, ...) {}
inline void debugN(const char *s, ...) {}
inline void debugN(int level, const char *s, ...) {}
inline void debugC(int level, uint32 debugChannel, const char *s, ...) {}
inline void debugC(uint32 debugChannel, const char *s, ...) {}
inline void debugCN(int level, uint32 debugChannel, const char *s, ...) {}
inline void debugCN(uint32 debugChannel, const char *s, ...) {}

#else

/**
 * @defgroup common_debug Debug functions
 * @ingroup common
 *
 * @brief  Functions for printing debug messages.
 * @{
 */

/**
 * Print a debug message to the text console (stdout).
 * Automatically appends a newline.
 */
void debug(MSVC_PRINTF const char *s, ...) GCC_PRINTF(1, 2);

/**
 * Print a debug message to the text console (stdout), but only if
 * the gDebugLevel equals at least the specified level.
 * As a rule of thumb, the more important the message, the lower the level.
 * Automatically appends a newline.
 */
void debug(int level, MSVC_PRINTF const char *s, ...) GCC_PRINTF(2, 3);

/**
 * Print a debug message to the text console (stdout).
 * Does not append a newline.
 */
void debugN(MSVC_PRINTF const char *s, ...) GCC_PRINTF(1, 2);

/**
 * Print a debug message to the text console (stdout), but only if
 * the gDebugLevel equals at least the specified level.
 * As a rule of thumb, the more important the message, the lower the level.
 * Does not append a newline.
 */
void debugN(int level, MSVC_PRINTF const char *s, ...) GCC_PRINTF(2, 3);

/**
 * Print a debug message to the text console (stdout), but only if
 * the gDebugLevel equals at least the specified level AND
 * if the specified special debug level is active.
 * As a rule of thumb, the more important the message, the lower the level.
 * Automatically appends a newline.
 * @see enableDebugChannel
 *
 * @param level         Debug level that must be active for the message to be printed.
 * @param debugChannel  Channel to check against.
 * @param s             Message to print.
 */
void debugC(int level, uint32 debugChannel, MSVC_PRINTF const char *s, ...) GCC_PRINTF(3, 4);

/**
 * Print a debug message to the text console (stdout), but only if
 * the gDebugLevel equals at least the specified level AND
 * if the specified special debug level is active.
 * As a rule of thumb, the more important the message, the lower the level.
 * Does not append a newline automatically.
 * @see enableDebugChannel
 *
 * @param level         Debug level that must be active for the message to be printed.
 * @param debugChannel  Channel to check against.
 * @param s             Message to print.
 *
 */
void debugCN(int level, uint32 debugChannel, MSVC_PRINTF const char *s, ...) GCC_PRINTF(3, 4);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified special debug level is active.
 * Automatically appends a newline.
 * @see enableDebugChannel
 *
 * @param debugChannel  Channel to check against.
 * @param s             Message to print.
 */
void debugC(uint32 debugChannel, MSVC_PRINTF const char *s, ...) GCC_PRINTF(2, 3);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified special debug level is active.
 * Does not append a newline automatically.
 * @see enableDebugChannel
 *
 * @param debugChannel  Channel to check against.
 * @param s             Message to print.
 */
void debugCN(uint32 debugChannel, MSVC_PRINTF const char *s, ...) GCC_PRINTF(2, 3);

#endif

/**
 * Check whether the debug level is set to the specified level.
 */
bool debugLevelSet(int level);

/**
 * Check whether the debug level and channel are active.
 *
 * @param level         Debug level to check against. If set to -1, only channel check is active.
 * @param debugChannel  Channel to check against.
 * @see enableDebugChannel
 */
bool debugChannelSet(int level, uint32 debugChannel);

/**
 * The debug level. Initially set to -1, indicating that no debug output
 * should be shown. Positive values usually imply that an increasing number of
 * debug output shall be generated. The higher the value, the more verbose the
 * information (although the exact semantics are up to the engines).
 */
extern int gDebugLevel;

/**
 * Specify whether to show only the debug channels and suppress
 * the non-channeled output.
 *
 * This option is useful when you want to have higher levels of channels
 * visible without the noise from other subsystems or OSystem.
 */
extern bool gDebugChannelsOnly;

/** Global constant for EventRecorder debug channel. */
enum GlobalDebugLevels {
	kDebugGlobalDetection = 100000,
	kDebugLevelEventRec,
	kDebugLevelMainGUI,
	kDebugLevelMacGUI,
	kDebugLevelGGraphics,
	kDebugLevelGVideo,
};

/** @} */

#endif
