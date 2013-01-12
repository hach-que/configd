#ifndef __HIVE_INOTIFY_H
#define __HIVE_INOTIFY_H

#include <bstrlib.h>
#include "hive_app.h"

void hive_inotify_register(app_t* app);
void hive_inotify_poll(app_t* app);
void hive_inotify_set_callback_updated(app_t* app, void (*updated)(app_t* app, bstring path));
void hive_inotify_set_callback_deleted(app_t* app, void (*deleted)(app_t* app, bstring path));

#endif