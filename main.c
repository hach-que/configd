#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <bstrlib.h>
#include "hive_yaml.h"
#include "hive_app.h"

int main(int argc, char** argv)
{
    bstring etc_path = bfromcstr("/etc/configd");
    bstring mount_path = bfromcstr("/etc");
    
    // TODO: Use argtable2.
    if (argc != 1 && argc != 3)
    {
        printf("invalid arguments.\n");
        return 1;
    }
    else if (argc == 3)
    {
        etc_path = bfromcstr(argv[1]);
        mount_path = bfromcstr(argv[2]);
    }
    
    // Open a reference to the configuration directory, as our mountpoint
    // may hide it.
    app_t app;
    app.active.path = mount_path;
    app.active.content = opendir((const char*)app.active.path);
    app.source.path = etc_path;
    app.source.content = opendir((const char*)app.source.path);
    
    app_init(&app);
    app_run(&app);
}