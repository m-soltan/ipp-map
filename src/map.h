/** @file
 * Interface of a class storing a map of trunk roads (Routes)
 *
 * @author Łukasz Kamiński <kamis@mimuw.edu.pl>, Marcin Peczarski <marpe@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 20.03.2019
 */

#ifndef MAP_MAP_H
#define MAP_MAP_H

#include <stdbool.h>
#include <stdlib.h>
#include "global_declarations.h"

/** @brief Create a new map structure.
 * Create a new, empty structure with no cities, roads or routes
 * @return Pointer to the structure created or NULL if memory allocation has
 * failed
 */
Map *newMap(void);

/** @brief Remove the structure.
 * Remove the structure @p map references.
 * Do nothing when a NULL argument is passed.
 * @param[in] map        – pointer to the structure being removed.
 */
void deleteMap(Map *map);

/** @brief Add a road section between two distinct cities.
 * Either city will be added to the map if necessary, then a road is created
 * between the two cities.
 * @param[in,out] map    – pointer to the road map structure;
 * @param[in] city1      – pointer to the city name as a string;
 * @param[in] city2      – pointer to the city name as a string;
 * @param[in] length     – road length in kilometers;
 * @param[in] builtYear  – the year the road was built.
 * @return @p true if the road was added.
 * @p false, if an error occurred: a parameter has an invalid value, both
 * names are identical, the two cities already have a road between them or
 * memory allocation failed.
 */
bool addRoad(Map *map, const char *city1, const char *city2,
		unsigned length, int builtYear);

/** @brief Modify the year the road was last repaired.
 * If the road section was already repaired, the repair year will be changed.
 * Otherwise, a repair year will be set.
 * @param[in,out] map    – pointer to the road map structure;
 * @param[in] city1      – pointer to the city name as a string;
 * @param[in] city2      – pointer to the city name as a string;
 * @param[in] repairYear – the year of the repair to be recorded
 * @return @p true if the change was successful.
 *  @p false, if an error occurred: a parameter has an invalid value, both
 * names are identical, one of the cities does not exist, there is no road
 * between the cities, the new repair is earlier than construction or a
 * previous repair.
 */
bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear);

/** @brief Create a Route as described by the list.
 * Use the lists passed as parameters to build a Route.
 * @param[in,out] map    – pointer to the road map structure;
 * @param[in] id         – new Route number
 * @param names          – names of all cities used by the road
 * @param rLengths       – lengths of the roads the Route uses
 * @param years          – last repair or construction years of the roads
 * @param length         – number of cities the Route will go through
 * @return @p true if the Route was successfully created
 * @p false if an error occurred: a parameter has an invalid value, a list
 * contains an invalid value, this id is already taken or memory allocation
 * has failed.
 */
bool routeFromList(Map *map, unsigned id, const char **names,
		const unsigned *rLengths, const int *years, size_t length);

/** @brief Create a Route from one city to the other.
 * Create a Route with the id given. It starts in @p city1 and ends
 * in @p city2. The route is a path in the road map. Firstly, it can't be
 * longer than any other path between these cities. Secondly, it can't
 * use a road that was last repaired (or built if it was never repaired)
 * earlier than all roads on some alternative path of equal length. Thirdly,
 * it must be uniquely determined.
 * @param[in,out] map    – pointer to the road map structure;
 * @param[in] routeId    – new Route number
 * @param[in] city1      – pointer to the city name as a string;
 * @param[in] city2      – pointer to the city name as a string;
 * @return @p false if an error occurred: a parameter has an invalid value,
 * the id is already taken by a different Route, the names don't describe a
 * valid pair of cities, there is no matching path or memory allocation failed.
 */
bool newRoute(Map *map, unsigned routeId,
		const char *city1, const char *city2);

/** @brief Extend the Route so that it ends in the given city.
 * Append new roads to the route. The added segments must form a path. Firstly,
 * the path can't be longer than any other valid path between these cities.
 * Secondly, it can't use a road that was last repaired (or built if it was
 * never repaired) earlier than all roads on some alternative path of equal
 * length. Thirdly, it must be uniquely determined. Only the paths that can be
 * used to extend the Route into a different, valid Route are considered for
 * the above conditions.
 * @param[in,out] map    – pointer to the road map structure;
 * @param[in] routeId    – Route number
 * @param[in] city       – pointer to the city name as a string;
 * @return @p true if the extension was successful
 * @p false if an error occurred: a parameter has an invalid value, no route
 * with the given id exists, no city with such a name exists, the city already
 * is a part of this Route, there is no valid extension or memory allocation
 * failed.
 */
bool extendRoute(Map *map, unsigned routeId, const char *city);

/** @brief Remove the road between the two cities.
 * Removes the road. If the road is a part of a Route, a detour will be created
 * from existing roads to replace the missing section. The detour must be a
 * path in the road map graph. Firstly, it can't be
 * longer than any other path between these cities. Secondly, it can't
 * use a road that was last repaired (or built if it was never repaired)
 * earlier than all roads on some alternative path of equal length. Thirdly,
 * it must be uniquely determined. Only the paths that can be
 * used as a detour to rebuild the Route into a different, valid Route are
 * considered for the above conditions.
 * @param[in,out] map    – pointer to the road map structure;
 * @param[in] city1      – pointer to the city name as a string;
 * @param[in] city2      – pointer to the city name as a string;
 * @return @p true if the removal succeeded
 * @p false if an error prevented the removal: a parameter has an invalid
 * value, the names don't refer to a valid pair of cities, a detour was
 * needed but couldn't be created or memory allocation failed.
 */
bool removeRoad(Map *map, const char *city1, const char *city2);

/** @brief Remove the Route using this id.
 * Removes the Route from the road map structure so that a new Route with this
 * id can be created.
 * @param[in,out] map    – pointer to the road map structure;
 * @param[in] routeId    – Route number
 * @return @p true if removal was successful
 * @p false if the passed integer isn't an id of an existing Route.
 */
bool removeRoute(Map *map, unsigned routeId);

/** @brief Describes a Route, returns a modifiable string.
 * Returns a string describing a Route. Memory is allocated for the description
 * and must be released using the free function.
 * The format of this description is:
 * Route number;city name;road length;construction or last repair year;city
 * name;road length;construction or last repair year;city name;road
 * length;construction or last repair year;...;city name.
 * The starting city is chosen so that the order of the cities from the Route's
 * creation is preserved
 * @return a string containing the description on success, NULL on memory
 * allocation failure, empty string on any other error.
 */
const char * getRouteDescription(Map *map, unsigned routeId);


/** @brief Describes a Route, returns a non-modifiable string.
 * Returns a string describing a Route. Memory is allocated for the description
 * and must be released using the free function.
 * The format of this description is:
 * Route number;city name;road length;construction or last repair year;city
 * name;road length;construction or last repair year;city name;road
 * length;construction or last repair year;...;city name.
 * The starting city is chosen so that the order of the cities from the Route's
 * creation is preserved
 * @return a string containing the description on success, NULL on memory
 * allocation failure, empty string on any other error.
 */
char *routeDescriptionAux(Map *map, unsigned routeId);

#endif /* MAP_MAP_H */
