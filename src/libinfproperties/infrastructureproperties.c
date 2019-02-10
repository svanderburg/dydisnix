#include "infrastructureproperties.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <glib/gprintf.h>

#define BUFFER_SIZE 1024

gchar *generate_infrastructure_xml_from_expr(char *infrastructure_expr)
{
    int pipefd[2];
    
    if(pipe(pipefd) == 0)
    {
        int status = fork();
        
        if(status == -1)
        {
            g_printerr("Error with forking dydisnix-xml process!\n");
            return NULL;
        }
        else if(status == 0)
        {
            char *const args[] = { "dydisnix-xml", "-i", infrastructure_expr, NULL };
            
            close(pipefd[0]); /* Close read-end of pipe */
            dup2(pipefd[1], 1);
            execvp("dydisnix-xml", args);
            _exit(1);
        }
        else
        {
            char line[BUFFER_SIZE];
            ssize_t line_size;
        
            close(pipefd[1]); /* Close write-end of pipe */
            
            line_size = read(pipefd[0], line, BUFFER_SIZE - 1);
            line[line_size - 1] = '\0'; /* Replace linefeed char with termination */

            close(pipefd[0]);
            
            wait(&status);
            
            if(WEXITSTATUS(status) == 0)
                return g_strdup(line);
            else
                return NULL;
        }
    }
    else
    {
        g_printerr("Error with creating a pipe\n");
        return NULL;
    }
}

GPtrArray *create_target_array_from_xml(const gchar *infrastructure_xml_file)
{
    GPtrArray *targets_array;
    xmlDocPtr doc;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(infrastructure_xml_file)) == NULL)
    {
	g_printerr("Error with parsing the infrastructure XML file!\n");
	xmlCleanupParser();
	return NULL;
    }
    
    /* Create a target array from the XML document */
    targets_array = create_target_array_from_doc(doc);
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    /* Return the target array */
    return targets_array;
}

static gint compare_target_names(const Target **l, const Target **r)
{
    const Target *left = *l;
    const Target *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

Target *find_target_by_name(GPtrArray *target_array, gchar *name)
{
    Target target;
    Target **ret, *targetPtr = &target;
    
    targetPtr->name = name;
    
    ret = bsearch(&targetPtr, target_array->pdata, target_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_target_names);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

static gint compare_target_property_keys(const TargetProperty **l, const TargetProperty **r)
{
    const TargetProperty *left = *l;
    const TargetProperty *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

TargetProperty *find_target_property(Target *target, gchar *name)
{
    if(target->properties == NULL)
        return NULL;
    else
    {
        TargetProperty prop;
        TargetProperty **ret, *propPtr = &prop;
        
        propPtr->name = name;
        
        ret = bsearch(&propPtr, target->properties->pdata, target->properties->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_target_property_keys);
        
        if(ret == NULL)
            return NULL;
        else
            return *ret;
    }
}

void substract_target_value(Target *target, gchar *property_name, int amount)
{
    gchar buffer[BUFFER_SIZE];
    TargetProperty *property = find_target_property(target, property_name);
    int value = atoi(property->value) - amount;
    g_sprintf(buffer, "%d", value);
    g_free(property->value);
    property->value = g_strdup(buffer);
}
