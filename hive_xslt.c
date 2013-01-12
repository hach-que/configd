#include <assert.h>
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>
#include "hive_object.h"
#include "hive_xslt.h"

bstring hive_xslt_object_to_xml_impl(struct object* object)
{
    bstring result = bfromcstr("");
    switch (object->type)
    {
        case OBJECT_TYPE_NIL:
            return result;
        case OBJECT_TYPE_NUMBER:
            // FIXME: When OBJECT_TYPE_NUMBER is actually used...
            return result;
        case OBJECT_TYPE_STRING:
        {
            bcatcstr(result, "<string>");
            bconcat(result, object->string);
            bcatcstr(result, "</string>");
            return result;
        }
        case OBJECT_TYPE_LIST:
        {
            bcatcstr(result, "<list>");
            list_iterator_start(&object->list);
            while (list_iterator_hasnext(&object->list))
            {
                bstring child = hive_xslt_object_to_xml_impl(list_iterator_next(&object->list));
                bconcat(result, child);
                bdestroy(child);
            }
            list_iterator_stop(&object->list);
            bcatcstr(result, "</list>");
            return result;
        }
        case OBJECT_TYPE_MAP:
        {
            bcatcstr(result, "<map>");
            list_iterator_start(&object->map);
            while (list_iterator_hasnext(&object->map))
            {
                struct map_entry* entry = list_iterator_next(&object->map);
                bstring key = hive_xslt_object_to_xml_impl(entry->key);
                bstring value = hive_xslt_object_to_xml_impl(entry->value);
                bstring child = bformat("<entry><key>%s</key><value>%s</value></entry>", key->data, value->data);
                bconcat(result, child);
                bdestroy(key);
                bdestroy(value);
                bdestroy(child);
            }
            list_iterator_stop(&object->map);
            bcatcstr(result, "</map>");
            return result;
        }
        default:
            assert(false);
            return NULL;
    }
}

bstring hive_xslt_object_to_xml(struct object* object)
{
    bstring result = bfromcstr("<?xml version=\"1.0\" ?><configuration>");
    bstring result_impl = hive_xslt_object_to_xml_impl(object);
    bconcat(result, result_impl);
    bcatcstr(result, "</configuration>");
    bdestroy(result_impl);
    return result;
}

int hive_xslt_write_xml_to_bstring(void* context, const char* buffer, int len)
{
    bcatcstr(context, buffer);
    return len;
}

void hive_xslt_transform_with_path_to_file(bstring xslt_path, bstring xml_data, bstring output_path)
{
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    xsltStylesheetPtr xslt_doc = xsltParseStylesheetFile((const xmlChar*)xslt_path->data);
    if (xslt_doc == NULL)
    {
        fprintf(stderr, "invalid xslt: %s\n", xslt_path->data);
        return;
    }
    xmlDocPtr xml_doc = xmlReadMemory((const char*)xml_data->data, blength(xml_data), "unnamed.xml", NULL, 0);
    if (xml_doc == NULL)
    {
        fprintf(stderr, "invalid internal generation for: %s\n", output_path->data);
        return;
    }
    xmlDocPtr xml_result = xsltApplyStylesheet(xslt_doc, xml_doc, NULL);
    if (xml_result == NULL)
    {
        fprintf(stderr, "invalid application of xslt: %s\n", xslt_path->data);
        return;
    }
    xsltSaveResultToFilename((const char*)output_path->data, xml_result, xslt_doc, 0);
    xsltFreeStylesheet(xslt_doc);
    xmlFreeDoc(xml_doc);
    xmlFreeDoc(xml_result);
}