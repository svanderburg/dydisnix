#include "docs-config.h"
#include <xmlutil.h>
#include <procreact_future.h>

static ProcReact_Future generate_docs_xml_from_expr_async(char *docs_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "--docs", docs_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_docs_xml_from_expr(char *docs_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_docs_xml_from_expr_async(docs_expr);
    char *path = procreact_future_get(&future, &status);
    path[strlen(path) - 1] = '\0';
    return path;
}

static gint compare_group_keys(const Group **l, const Group **r)
{
    const Group *left = *l;
    const Group *right = *r;

    return g_strcmp0(left->name, right->name);
}

DocsConfig *create_docs_config_from_xml(gchar *docs_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;

    DocsConfig *docs_config = (DocsConfig*)g_malloc(sizeof(DocsConfig));
    docs_config->groups = g_ptr_array_new();

    /* Parse the XML document */

    if((doc = xmlParseFile(docs_xml_file)) == NULL)
    {
        g_printerr("Error with parsing the services XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The docs XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    result = executeXPathQuery(doc, "/docs-config/groups/group");

    if(result)
    {
        unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;

        for(i = 0; i < nodeset->nodeNr; i++)
        {
            xmlAttr *group_properties = nodeset->nodeTab[i]->properties;
            xmlNodePtr group_children = nodeset->nodeTab[i]->children;
            Group *group = (Group*)g_malloc(sizeof(Group));
            group->name = NULL;
            group->value = NULL;

            /* Read name attribute */
            while(group_properties != NULL)
            {
                if(xmlStrcmp(group_properties->name, (const xmlChar*) "name") == 0)
                    group->name = g_strdup((gchar*)group_properties->children->content);

                group_properties = group_properties->next;
            }

            if(group_children != NULL)
                group->value = g_strdup((gchar*)group_children->content);

            g_ptr_array_add(docs_config->groups, group);
        }
    }

    g_ptr_array_sort(docs_config->groups, (GCompareFunc)compare_group_keys);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the docs configuration */
    return docs_config;
}

DocsConfig *create_docs_config_from_nix(gchar *docs_nix)
{
    char *docs_xml = generate_docs_xml_from_expr(docs_nix);
    DocsConfig *docs_config = create_docs_config_from_xml(docs_xml);
    free(docs_xml);
    return docs_config;
}

DocsConfig *create_docs_config(gchar *docs, const int xml)
{
    if(xml)
        return create_docs_config_from_xml(docs);
    else
        return create_docs_config_from_nix(docs);
}

void delete_docs_config(DocsConfig *docs_config)
{
    if(docs_config != NULL)
    {
        unsigned int i;

        for(i = 0; i< docs_config->groups->len; i++)
        {
            Group *group = g_ptr_array_index(docs_config->groups, i);
            g_free(group->name);
            g_free(group->value);
            g_free(group);
        }

        g_free(docs_config);
    }
}

gchar *find_group(const DocsConfig *docs_config, gchar *name)
{
    Group group;
    Group **ret, *groupPtr = &group;

    groupPtr->name = name;

    ret = bsearch(&groupPtr, docs_config->groups->pdata, docs_config->groups->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_group_keys);

    if(ret == NULL)
        return NULL;
    else
        return (*ret)->value;
}
