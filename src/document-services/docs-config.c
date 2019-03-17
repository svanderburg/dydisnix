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

static DocsConfig *parse_docs_config(xmlNodePtr element)
{
    DocsConfig *docs_config = (DocsConfig*)g_malloc0(sizeof(DocsConfig));
    xmlNodePtr element_children = element->children;

    while(element_children != NULL)
    {
        if(xmlStrcmp(element_children->name, (xmlChar*) "groups") == 0)
            docs_config->groups = parse_dictionary_attr(element_children, "group", "name", parse_value);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "fields") == 0)
            docs_config->fields = parse_list(element_children, "field", parse_value);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "descriptions") == 0)
            docs_config->descriptions = parse_dictionary(element_children, parse_value);

        element_children = element_children->next;
    }

    return docs_config;
}

DocsConfig *create_docs_config_from_xml(gchar *docs_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    DocsConfig *docs_config;

    /* Parse the XML document */

    if((doc = xmlParseFile(docs_xml_file)) == NULL)
    {
        g_printerr("Error with parsing the docs XML file!\n");
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

    /* Parse groups */
    docs_config = parse_docs_config(node_root);

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
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, docs_config->groups);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            g_free(key);
            g_free(value);
        }

        g_hash_table_destroy(docs_config->groups);

        for(i = 0; i < docs_config->fields->len; i++)
        {
            gchar *field = g_ptr_array_index(docs_config->fields, i);
            g_free(field);
        }

        g_ptr_array_free(docs_config->fields, TRUE);

        g_hash_table_iter_init(&iter, docs_config->descriptions);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            g_free(key);
            g_free(value);
        }

        g_hash_table_destroy(docs_config->descriptions);

        g_free(docs_config);
    }
}

gchar *find_group(const DocsConfig *docs_config, gchar *name)
{
    return g_hash_table_lookup(docs_config->groups, name);
}

gchar *find_description(const DocsConfig *docs_config, gchar *name)
{
    return g_hash_table_lookup(docs_config->descriptions, name);
}
