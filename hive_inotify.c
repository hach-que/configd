///
/// @file
/// @brief Provides functionality to monitor configuration sources.
/// @author James Rhodes
///
/// This file provides functions and callbacks that monitor the configuration
/// source directory so that files can be processed when they are updated.
///

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <simclist.h>
#include <dirent.h>
#include "hive_inotify.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

///
/// @brief A pair structure with the inotify watch descriptor and the actual file path.
///
struct inotify_watch
{
    ///
    /// @brief The watch descriptor.
    ///
    int wd;
    
    ///
    /// @brief The path that this watch descriptor matches up to.
    ///
    bstring path;
};

///
/// @brief Returns the size of the elements in the list of inotify pairs.
///
/// @param el The current element.
///
size_t inotify_watch_meter(const void* el)
{
    return sizeof(struct inotify_watch);
}

///
/// @brief Finds an element in the inotify pair list based on watch descriptor.
///
/// @param el The current element that is being found.
/// @param key A pointer to the watch descriptor integer.
///
int inotify_watch_seeker(const void* el, const void* key)
{
    return ((struct inotify_watch*)el)->wd == *(int*)key;
}

///
/// @brief Adds a new directory to the list of directories to watch.
///
/// @param app The application.
/// @param path The path of the directory to watch.
///
void hive_inotify_watch_add(app_t* app, bstring path)
{
    assert(path != NULL);
    struct inotify_watch watch;
    printf("--> %s\n", (const char*)path->data);
    watch.wd = inotify_add_watch(app->source.inotify, (const char*)path->data, IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE);
    watch.path = bstrcpy(path);
    list_append(&app->source.watches, &watch);
}

///
/// @brief Removes a directory from the list of directories to watch.
///
/// @param app The application.
/// @param path The path of the directory to stop watching.
///
void hive_inotify_watch_remove(app_t* app, bstring path)
{
    struct inotify_watch* watch = NULL;
    list_iterator_start(&app->source.watches);
    while (list_iterator_hasnext(&app->source.watches))
    {
        struct inotify_watch* potential = list_iterator_next(&app->source.watches);
        if (biseq(path, potential->path))
        {
            watch = potential;
            break;
        }
    }
    list_iterator_stop(&app->source.watches);
    if (watch == NULL)
        return; // We weren't being notified of this directory anyway.
    inotify_rm_watch(app->source.inotify, watch->wd);
    printf("<-- %s\n", (const char*)watch->path->data);
    bdestroy(watch->path);
    list_delete(&app->source.watches, watch);
}

///
/// @internal
/// @brief Recursively registers directories.
///
void hive_inotify_register_recursive(app_t* app, bstring path)
{
    hive_inotify_watch_add(app, path);
    DIR* dir = opendir((const char*)path->data);
    if (!dir) return;
    while (true)
    {
        struct dirent* entry;
        entry = readdir(dir);
        if (!entry) return;
        if (entry->d_type & DT_DIR)
        {
            if (strcmp(entry->d_name, "..") == 0 ||
                strcmp(entry->d_name, ".") == 0)
                continue;
            bstring child = bstrcpy(path);
            bconchar(child, '/');
            bcatcstr(child, entry->d_name);
            hive_inotify_register_recursive(app, child);
            bdestroy(child);
        }
    }
}

///
/// @brief Registers inotify events.
///
void hive_inotify_register(app_t* app)
{
    // Initialize the list that we use for mapping watches to their parents.
    list_init(&app->source.watches);
    list_attributes_copy(&app->source.watches, inotify_watch_meter, true);
    list_attributes_seeker(&app->source.watches, inotify_watch_seeker);
    
    // Initialize inotify and monitor the source configuration directory.
    app->source.inotify = inotify_init();
    hive_inotify_register_recursive(app, app->source.path);
}

///
/// @brief Performs a single non-blocking poll of inotify events.
///
void hive_inotify_poll(app_t* app)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    // Check to see if there are any more events.
    fd_set iwatch;
    FD_ZERO(&iwatch);
    FD_SET(app->source.inotify, &iwatch);
        
    // Attempt to read from the descriptor.
    int retval = select(app->source.inotify + 1, &iwatch, NULL, NULL, &timeout);
    if (retval == -1)
        fprintf(stderr, "error while using select()\n");
    else if (FD_ISSET(app->source.inotify, &iwatch))
    {
        char buffer[EVENT_BUF_LEN];
        size_t length = read(app->source.inotify, buffer, EVENT_BUF_LEN);
        int ii = 0;
        while (ii < length)
        {
            // Get the event and the watch data.
            struct inotify_event* event = (struct inotify_event*)&buffer[ii];
            struct inotify_watch* watch = list_seek(&app->source.watches, &event->wd);
            
            // Construcwatcht a joined name automatically.
            bstring joined = bstrcpy(watch->path);
            bconchar(joined, '/');
            bcatcstr(joined, event->name);
            
            // Check what type event it was and handle it.
            if ((event->mask & IN_CREATE) || (event->mask & IN_MOVED_TO))
            {
                if (event->mask & IN_ISDIR)
                    hive_inotify_watch_add(app, joined);
                else if (app->source.updated != NULL)
                    app->source.updated(app, joined);
            }
            else if ((event->mask & IN_DELETE) || (event->mask & IN_DELETE_SELF) || (event->mask & IN_MOVED_FROM))
            {
                if (event->mask & IN_ISDIR)
                    hive_inotify_watch_remove(app, joined);
                else if (app->source.deleted != NULL)
                    app->source.deleted(app, joined);
            }
            else if ((event->mask & IN_CLOSE_WRITE) && !(event->mask & IN_ISDIR))
            {
                if (app->source.updated != NULL)
                    app->source.updated(app, joined);
            }
            
            // Free data.
            bdestroy(joined);
            
            ii += EVENT_SIZE + event->len;
        }
    }
}

///
/// @brief Sets the callback function for when a YAML file is created or updated.
///
/// @param app The main application.
/// @param updated The callback function.
///
void hive_inotify_set_callback_updated(app_t* app, void (*updated)(app_t* app, bstring path))
{
    app->source.updated = updated;
}

///
/// @brief Sets the callback function for when a YAML file is deleted.
///
/// @param app The main application.
/// @param updated The callback function.
///
void hive_inotify_set_callback_deleted(app_t* app, void (*deleted)(app_t* app, bstring path))
{
    app->source.deleted = deleted;
}
