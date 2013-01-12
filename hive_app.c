#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "hive_fuse.h"
#include "hive_app.h"
#include "hive_inotify.h"
#include "hive_yaml.h"
#include "hive_xslt.h"
    
struct path_info
{
    bool is_valid;
    bstring yaml;
    bstring xslt;
    bstring output;
};

///
/// @internal
/// @brief Gets both the YAML and XSLT path information based on a single path.
///
struct path_info get_path_info(app_t* app, bstring path)
{
    struct path_info result;
    bstring trailing4 = bmidstr(path, blength(path) - 4, 4);
    bstring trailing5 = bmidstr(path, blength(path) - 5, 5);
    bstring relative = bmidstr(path, blength(app->source.path) + 1, blength(path) - blength(app->source.path) - 1);
    if (path->data[0] == '.' || (!biseqcstrcaseless(trailing4, ".yml") && !biseqcstrcaseless(trailing5, ".xslt")))
    {
        result.is_valid = false;
        result.xslt = NULL;
        result.yaml = NULL;
        result.output = NULL;
    }
    else if (biseqcstrcaseless(trailing4, ".yml"))
    {
        result.is_valid = true;
        result.yaml = bstrcpy(path);
        result.xslt = bmidstr(path, 0, blength(path) - 4);
        bcatcstr(result.xslt, ".xslt");
        result.output = bmidstr(relative, 0, blength(relative) - 4);
        binsertch(result.output, 0, 1, '/');
        binsert(result.output, 0, app->active.path, ' ');
    }
    else if (biseqcstrcaseless(trailing5, ".xslt"))
    {
        result.is_valid = true;
        result.xslt = bstrcpy(path);
        result.yaml = bmidstr(path, 0, blength(path) - 5);
        bcatcstr(result.yaml, ".yml");
        result.output = bmidstr(relative, 0, blength(relative) - 5);
        binsertch(result.output, 0, 1, '/');
        binsert(result.output, 0, app->active.path, ' ');
    }
    else
        assert(false);
    bdestroy(trailing4);
    bdestroy(trailing5);
    bdestroy(relative);
    return result;
}

void app_on_updated(app_t* app, bstring path)
{
    struct path_info info = get_path_info(app, path);
    if (!info.is_valid)
        return;
    
    // Parse the YAML file.
    struct object* yaml = hive_yaml_parse_file(info.yaml);
    if (yaml == NULL)
    {
        fprintf(stderr, "missing yaml: %s\n", info.yaml->data);
        return;
    }
    
    // Convert the object to source XML.
    bstring xml = hive_xslt_object_to_xml(yaml);
    printf("%s", xml->data);
    
    // Parse and apply stylesheet, and save to the output.
    hive_xslt_transform_with_path_to_file(info.xslt, xml, info.output);
}

void app_on_deleted(app_t* app, bstring path)
{
    struct path_info info = get_path_info(app, path);
    if (!info.is_valid)
        return;
    
    // Delete the file in the active configuration directory.
    unlink((const char*)info.output->data);
}

///
/// @brief Initializes the main application.
///
void app_init(app_t* app)
{
    struct dirent de;
    
    // This might end up being a compile-time option.
    app->enable_fuse = false;
    
    // Register inotify.
    hive_inotify_register(app);
    
    // Set inotify callbacks.
    hive_inotify_set_callback_updated(app, &app_on_updated);
    hive_inotify_set_callback_deleted(app, &app_on_deleted);
}

///
/// @brief Runs the main application loop.
///
void app_run(app_t* app)
{
    // Handle inotify.
    while (true)
        hive_inotify_poll(app);
}