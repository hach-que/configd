#include <assert.h>
#include <stdbool.h>
#include <yaml.h>
#include "hive_yaml.h"
#include "hive_object.h"

struct object* hive_yaml_parse(yaml_parser_t* parser, yaml_event_t* event);

struct object* hive_yaml_parse_sequence(yaml_parser_t* parser)
{
    yaml_event_t event;
    
    if (!yaml_parser_parse(parser, &event))
        return NULL;
    if (event.type == YAML_SEQUENCE_END_EVENT)
        return NULL;
    
    return hive_yaml_parse(parser, &event);
}

struct map_entry* hive_yaml_parse_mapping(yaml_parser_t* parser)
{
    yaml_event_t event;
    
    if (!yaml_parser_parse(parser, &event))
        return NULL;
    if (event.type == YAML_MAPPING_END_EVENT)
        return NULL;
    
    struct map_entry* entry = malloc(sizeof(struct map_entry));
    memset(entry, 0, sizeof(struct map_entry));
    entry->key = hive_yaml_parse(parser, &event);
    if (!yaml_parser_parse(parser, &event) || event.type == YAML_MAPPING_END_EVENT)
    {
        hive_object_free(entry->key);
        free(entry);
        return NULL;
    }
    entry->value = hive_yaml_parse(parser, &event);
    return entry;
}

struct object* hive_yaml_parse(yaml_parser_t* parser, yaml_event_t* event)
{
    switch (event->type)
    {
        case YAML_STREAM_END_EVENT:
        case YAML_DOCUMENT_END_EVENT:
        {
            struct object* result = malloc(sizeof(struct object));
            memset(result, 0, sizeof(struct object));
            result->type = OBJECT_TYPE_NIL;
            return result;
        }
        case YAML_DOCUMENT_START_EVENT:
        case YAML_STREAM_START_EVENT:
        {
            yaml_event_t event;
            if (!yaml_parser_parse(parser, &event))
                return NULL;
            return hive_yaml_parse(parser, &event);
        }
        case YAML_MAPPING_START_EVENT:
        {
            struct object* result = malloc(sizeof(struct object));
            memset(result, 0, sizeof(struct object));
            result->type = OBJECT_TYPE_MAP;
            list_init(&result->map);
            struct map_entry* entry = hive_yaml_parse_mapping(parser);
            while (entry != NULL)
            {
                list_append(&result->map, entry);
                entry = hive_yaml_parse_mapping(parser);
            }
            return result;
        }
        case YAML_SCALAR_EVENT:
        {
            struct object* result = malloc(sizeof(struct object));
            memset(result, 0, sizeof(struct object));
            result->type = OBJECT_TYPE_STRING;
            result->string = bfromcstr((const char*)event->data.scalar.value);
            return result;
        }
        case YAML_SEQUENCE_START_EVENT:
        {
            struct object* result = malloc(sizeof(struct object));
            memset(result, 0, sizeof(struct object));
            result->type = OBJECT_TYPE_LIST;
            list_init(&result->list);
            struct object* entry = hive_yaml_parse_sequence(parser);
            while (entry != NULL)
            {
                list_append(&result->list, entry);
                entry = hive_yaml_parse_sequence(parser);
            }
            return result;
        }
        default:
        {
            printf("%u\n", event->type);
            assert(false);
        }
    }
}

///
/// @brief Reads in a YAML file and returns an object result.
///
/// @param path The path to read from.
/// @return The resulting object structure.
///
struct object* hive_yaml_parse_file(bstring path)
{
    yaml_event_t event;
    yaml_parser_t parser;
    
    struct object* result = malloc(sizeof(struct object*));

    int done = 0;
    FILE* file = fopen((const char*)path->data, "rb");
    if (file == NULL)
        return NULL;
    bstring content = bread(&fread, file);
    fclose(file);
    
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser, content->data, blength(content));
    
    if (!yaml_parser_parse(&parser, &event))
        return NULL;
    
    result = hive_yaml_parse(&parser, &event);
    
    yaml_parser_delete(&parser);
    bdestroy(content);
    
    return result;
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
