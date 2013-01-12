#include <stdlib.h>
#include <stdio.h>
#include "hive_object.h"

///
/// @internal
/// @brief Frees a map entry pair.
///
void hive_map_entry_free(struct map_entry* map_entry)
{
    hive_object_free(map_entry->key);
    hive_object_free(map_entry->value);
    free(map_entry);
}

///
/// @brief Frees an object and all of it's content.
///
/// @param object The object to free.  After this function, the pointer is invalid.
///
void hive_object_free(struct object* object)
{
    switch (object->type)
    {
        case OBJECT_TYPE_NIL:
            break;
        case OBJECT_TYPE_NUMBER:
            break;
        case OBJECT_TYPE_STRING:
            bdestroy(object->string);
            break;
        case OBJECT_TYPE_LIST:
            list_iterator_start(&object->list);
            while (list_iterator_hasnext(&object->list))
                hive_object_free(list_iterator_next(&object->list));
            list_iterator_stop(&object->list);
            list_destroy(&object->list);
            break;
        case OBJECT_TYPE_MAP:
            list_iterator_start(&object->map);
            while (list_iterator_hasnext(&object->map))
                hive_map_entry_free(list_iterator_next(&object->map));
            list_iterator_stop(&object->map);
            list_destroy(&object->map);
            break;
    }
    free(object);
}

///
/// @brief Prints out the structure of an object to stdout for debugging.
///
/// This function will pretty print an object to stdout.
///
/// @param object The object to print out.
/// @param indent The initial indentation to apply to the output.
///
void hive_object_print(struct object* object, bstring indent)
{
    switch (object->type)
    {
        case OBJECT_TYPE_NIL:
            printf("%snil\n", (const char*)indent->data);
            break;
        case OBJECT_TYPE_NUMBER:
            printf("%snumber\n", (const char*)indent->data);
            break;
        case OBJECT_TYPE_STRING:
            printf("%sstring: '%s'\n", (const char*)indent->data, object->string->data);
            break;
        case OBJECT_TYPE_LIST:
            printf("%slist:\n", (const char*)indent->data);
            list_iterator_start(&object->list);
            while (list_iterator_hasnext(&object->list))
            {
                bstring newindent = bstrcpy(indent);
                bcatcstr(newindent, "    ");
                hive_object_print(list_iterator_next(&object->list), newindent);
            }
            list_iterator_stop(&object->list);
            break;
        case OBJECT_TYPE_MAP:
            printf("%smap:\n", (const char*)indent->data);
            list_iterator_start(&object->map);
            while (list_iterator_hasnext(&object->map))
            {
                bstring newindent = bstrcpy(indent);
                bcatcstr(newindent, "    ");
                struct map_entry* entry = list_iterator_next(&object->map);
                printf("%s    (key)\n", (const char*)indent->data);
                hive_object_print(entry->key, newindent);
                newindent = bstrcpy(indent);
                bcatcstr(newindent, "    ");
                printf("%s    (value)\n", (const char*)indent->data);
                hive_object_print(entry->value, newindent);
            }
            list_iterator_stop(&object->map);
            break;
        default:
            printf("%sunknown\n", (const char*)indent->data);
            break;
    }
    bdestroy(indent);
}