#ifndef __HIVE_XSLT_H
#define __HIVE_XSLT_H

#include <bstrlib.h>
#include "hive_object.h"

bstring hive_xslt_object_to_xml(struct object* object);
void hive_xslt_transform_with_path_to_file(bstring xslt_path, bstring xml_data, bstring output_path);

#endif