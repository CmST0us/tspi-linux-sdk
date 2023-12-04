#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>

#include <pthread.h>

#include <libxml/parser.h>

#include "json-c/json.h"
#include "dbserver.h"

void parsingxml(char *xmlfile)
{
    xmlDocPtr doc;
    xmlNodePtr curNode = NULL;
    xmlNodePtr lowNode = NULL;

    doc = xmlReadFile(xmlfile, "UTF-8", XML_PARSE_NOBLANKS);

    curNode = xmlDocGetRootElement(doc);
    printf("begin\n");
    if (g_str_equal(curNode->name, "sqlcmds")) {
        curNode = curNode->xmlChildrenNode;
        while (curNode) {
            char *sql = (char*)XML_GET_CONTENT(curNode->xmlChildrenNode);
            printf("%s\n",sql);
            char *json_str = dbserver_sql(sql, DBSERVER_STORAGE_INTERFACE);
            printf("%s\n", json_str);
            if (json_str)
                g_free(json_str);
            curNode = curNode->next;
        }
    }
    printf("end\n");
}

int main( int argc , char ** argv)
{
    if (argc == 2)
        parsingxml(argv[1]);

    return 0;
}
