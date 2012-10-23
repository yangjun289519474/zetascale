cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
#
IF($ENV{SCHOONER_VERSION})
	SET(VERSION $ENV{SCHOONER_VERSION})
ELSE()
	SET(VERSION 4.2)
ENDIF()
#
OPTION(OPTIMIZE "Generate optimized build" ON)
OPTION(DEBUG "Generate debug build" OFF)
OPTION(BUILD_SHARED "Generate shared libs" OFF)
OPTION(PERMISSIVE "Omit -Werror for build" OFF)
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty_inc)
#
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib)
#
IF(DEBUG)
	ADD_DEFINITIONS(-g3 -ggdb3)
	SET(OPTIMIZE OFF)
	SET(LIBRARY fdfdbg)
ENDIF(DEBUG)
#
IF(OPTIMIZE)
	ADD_DEFINITIONS(-D_OPTIMIZE)
	SET(LIBRARY fdf)
ENDIF(OPTIMIZE)
#
IF(BUILD_SHARED)
	ADD_DEFINITIONS(-fPIC)
	SET(LIB_TYPE SHARED)
ENDIF(BUILD_SHARED)
#
add_definitions(-O3 -pipe -Wall -fstack-protector-all -fno-strict-aliasing
-fno-omit-frame-pointer)
add_definitions( -D_GNU_SOURCE -DMPI_BUILD -DFLASH_SIM_TYPE=5
-DPLAT_LOG_COMPILE_LEVEL=DEVEL -DENABLE_MULTIPLE_FLASH_SUBSYSTEMS
-DBLD_VERSION=857 -DSDFAPI -DSDFAPIREFORMAT -DSIMPLE_REPLICATION
-DVIPGROUP_SUPPORT -DENABLE_MULTIPLE_FLASH_SUBSYSTEMS)
IF(NOT PERMISSIVE)
	add_definitions(-Werror)
ENDIF(NOT PERMISSIVE)
#
add_subdirectory(agent)
add_subdirectory(api)
add_subdirectory(ecc)
add_subdirectory(fth)
add_subdirectory(misc)
add_subdirectory(platform)
add_subdirectory(protocol)
add_subdirectory(sdftcp)
add_subdirectory(sdfmsg) 
add_subdirectory(utils)
add_subdirectory(ssd)
add_subdirectory(shared)
#
ADD_LIBRARY(${LIBRARY} ${LIB_TYPE} 
$<TARGET_OBJECTS:agent>
$<TARGET_OBJECTS:api>
$<TARGET_OBJECTS:ecc> 
$<TARGET_OBJECTS:fth> 
$<TARGET_OBJECTS:hotkey> 
$<TARGET_OBJECTS:misc> 
$<TARGET_OBJECTS:platform> 
$<TARGET_OBJECTS:protocol> 
$<TARGET_OBJECTS:action> 
$<TARGET_OBJECTS:home> 
$<TARGET_OBJECTS:replication> 
$<TARGET_OBJECTS:sdftcp> 
$<TARGET_OBJECTS:sdfmsg> 
$<TARGET_OBJECTS:utils> 
$<TARGET_OBJECTS:ssd> 
$<TARGET_OBJECTS:shared> 
$<TARGET_OBJECTS:clipper> 
$<TARGET_OBJECTS:fifo> 
)
#
IF(BUILD_SHARED)
	SET_TARGET_PROPERTIES(${LIBRARY} PROPERTIES 
	VERSION ${VERSION} 
	SOVERSION ${VERSION}
	)
ENDIF(BUILD_SHARED)
