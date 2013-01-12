#ifndef __HIVE_OBJECT_H
#define __HIVE_OBJECT_H

#include <bstrlib.h>
#include <simclist.h>

#define OBJECT_TYPE_NIL 0 ///< Indicates this object is a nil object.
#define OBJECT_TYPE_NUMBER 1 ///< Indicates this object is a number object.
#define OBJECT_TYPE_STRING 2 ///< Indicates this object is a string object.
#define OBJECT_TYPE_LIST 3 ///< Indicates this object is a list object.
#define OBJECT_TYPE_MAP 4 ///< Indicates this object is a map object.

///
/// @brief Represents a complex object (nil, number, string, list or map).
///
struct object
{
    int type; ///< The type of this object, one of the OBJECT_TYPE_* constants.   
    union
    {
        long number; ///< A numeric value.
        bstring string; ///< A string value.
        list_t list; ///< List of struct object.
        list_t map; ///< List of struct map_entry.
    };
};

///
/// @brief A map entry pair.
///
struct map_entry
{
    struct object* key; ///< The key of the map pair.
    struct object* value; ///< The value of the map pair.
};

void hive_object_free(struct object* object);
void hive_object_print(struct object* object, bstring indent);

#endif