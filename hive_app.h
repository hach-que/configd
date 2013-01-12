#ifndef __HIVE_APP_H
#define __HIVE_APP_H

#include <dirent.h>
#include <simclist.h>
#include <stdbool.h>
#include <bstrlib.h>

///
/// @brief A structure representing the configd application.
///
struct __app
{
    ///
    /// @brief Whether the FUSE module should be enabled.
    ///
    bool enable_fuse;
    
    ///
    /// @brief The active configuration information (often stored in /etc).
    ///
    struct
    {
        bstring path;
        DIR* content;
        int inotify;
        list_t watches;
        void (*updated)(struct __app* app, bstring path);
        void (*deleted)(struct __app* app, bstring path);
    } active;
    
    ///
    /// @brief The source configuration information (often stored in /etc/configd).
    ///
    struct
    {
        bstring path;
        DIR* content;
        int inotify;
        list_t watches;
        void (*updated)(struct __app* app, bstring path);
        void (*deleted)(struct __app* app, bstring path);
    } source;
};
typedef struct __app app_t;

void app_init(app_t* app);
void app_run(app_t* app);

#endif