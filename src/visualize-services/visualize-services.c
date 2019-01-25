#include "visualize-services.h"
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <serviceproperties.h>
#define BUFFER_SIZE 4096

static gchar *generate_service_xml_from_expr(char *service_expr)
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
            char *const args[] = { "dydisnix-xml", "-s", service_expr, NULL };
            
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

static void print_type(const Service *service)
{
    ServiceProperty *prop = find_service_property(service, "type");

    if(prop != NULL)
        g_print("\\n(%s)", prop->value);
}

static void generate_architecture_diagram(const GPtrArray *service_property_array)
{
    unsigned int i;

    g_print("digraph G {\n");
    g_print("node [style=filled,fillcolor=white,color=black];\n");

    /* Generate vertexes */
    for(i = 0; i < service_property_array->len; i++)
    {
         Service *current_service = g_ptr_array_index(service_property_array, i);
         g_print("\"%s\" [ label = \"%s", current_service->name, current_service->name);
         print_type(current_service);
         g_print("\" ]\n");
    }

    /* Generate edges */
    g_print("\n");

    for(i = 0; i < service_property_array->len; i++)
    {
         Service *current_service = g_ptr_array_index(service_property_array, i);
         unsigned int j;

         /* Inter-dependencies with ordering requirement */
         for(j = 0; j < current_service->depends_on->len; j++)
         {
             gchar *dependency = g_ptr_array_index(current_service->depends_on, j);
             g_print("\"%s\" -> \"%s\"\n", current_service->name, dependency);
         }

         /* Inter-dependencies without ordering requirement */
         for(j = 0; j < current_service->connects_to->len; j++)
         {
             gchar *dependency = g_ptr_array_index(current_service->connects_to, j);
             g_print("\"%s\" -> \"%s\" [style=dashed]\n", current_service->name, dependency);
         }
    }

    g_print("}\n");
}

int visualize_services(gchar *services, int xml)
{
    gchar *services_xml;
    GPtrArray *service_property_array;

    if(xml)
        services_xml = g_strdup(services);
    else
        services_xml = generate_service_xml_from_expr(services);

    service_property_array = create_service_property_array(services_xml);

    g_free(services_xml);

    if(service_property_array == NULL)
    {
        g_printerr("Cannot open services XML file!\n");
        return 1;
    }
    else
    {
        generate_architecture_diagram(service_property_array);
        delete_service_property_array(service_property_array);
        return 0;
    }
}
