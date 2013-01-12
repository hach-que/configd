configd
=============

configd is a prototype of a service that monitors a given directory and, using YAML and XSLT, transforms the raw configuration data into the configuration file for programs to read.

Why?
--------

By storing the raw configuration data in YAML and transforming it for the programs that need it, we can provide a consistent read / write setting API for configuring applications, without having to understand and parse the formats for every single application.  It's cleaner than bash shell scripts as the daemon automatically monitors the sources with inotify to ensure that the configuration is always up-to-date.

Areas for Expansion
-----------------------

  * Defining an API that programs can use to set and get configuration values.
  * Automatic reload / restart of systemd services when configuration files change.
    * Automatic restoration of previous configuration if the service does not start up under the new settings.

