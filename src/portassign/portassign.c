#include "portassign.h"
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"
#include "portconfiguration.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

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

static gchar *generate_infrastructure_xml_from_expr(char *infrastructure_expr)
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

static gchar *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr)
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
            char *const args[] = { "dydisnix-xml", "-i", infrastructure_expr, "-d", distribution_expr, NULL };
            
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

static gchar *generate_ports_xml_from_expr(char *ports_expr)
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
            char *const args[] = { "dydisnix-xml", "-p", ports_expr, NULL };
            
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

int portassign(gchar *services, gchar *infrastructure, gchar *distribution, gchar *ports, gchar *service_property, int xml)
{
    GPtrArray *service_property_array;
    GPtrArray *targets_array;
    GPtrArray *candidate_target_array;
    gchar *service_xml;
    gchar *infrastructure_xml;
    gchar *distribution_xml;
    gchar *ports_xml;

    if(xml)
    {
        service_xml = services;
        infrastructure_xml = infrastructure;
        distribution_xml = distribution;
        ports_xml = ports;
    }
    else
    {
        /* Convert Nix expressions to XML */
        service_xml = generate_service_xml_from_expr(services);
        infrastructure_xml = generate_infrastructure_xml_from_expr(infrastructure);
        distribution_xml = generate_distribution_xml_from_expr(distribution, infrastructure);
        
        if(ports == NULL)
            ports_xml = NULL;
        else
            ports_xml = generate_ports_xml_from_expr(ports);
    }

    service_property_array = create_service_property_array(service_xml);
    targets_array = create_target_array_from_xml(infrastructure_xml);
    candidate_target_array = create_candidate_target_array(distribution_xml);
    
    if(service_property_array == NULL || targets_array == NULL || candidate_target_array == NULL)
    {
        g_printerr("Error with opening one of the models!\n");
        return 1;
    }
    else
    {
        PortConfiguration port_configuration;
        unsigned int i;
        
        if(ports_xml == NULL)
            init_port_configuration(&port_configuration); /* If no ports config is given, initialise an empty one */
        else
            open_port_configuration(&port_configuration, ports_xml); /* Otherwise, open the ports config */
        
        /* Clean obsolete reservations */
        clean_obsolete_reservations(&port_configuration, candidate_target_array, service_property_array, service_property);
        
        g_print("{\n");
        g_print("  ports = {\n");
        
        for(i = 0; i < candidate_target_array->len; i++)
        {
            DistributionItem *distribution_item = g_ptr_array_index(candidate_target_array, i);
            Service *service = find_service(service_property_array, distribution_item->service);
            ServiceProperty *prop = find_service_property(service, service_property);
            
            /* For each service that has a port property that is unassigned, assign a port */

            if(prop != NULL)
            {
                if(g_strcmp0(prop->value, "shared") == 0) /* If a shared port is requested, consult the shared ports pool */
                {
                    gint port = assign_or_reuse_port(&port_configuration, NULL, distribution_item->service);
                    g_print("    %s = %d;\n", distribution_item->service, port);
                }
                else if(g_strcmp0(prop->value, "private") == 0) /* If a private port is requested, consult the machine's ports pool */
                {
                    if(distribution_item->targets->len > 0)
                    {
                        gchar *target = g_ptr_array_index(distribution_item->targets, 0);
                        gint port = assign_or_reuse_port(&port_configuration, target, distribution_item->service);
                        g_print("    %s = %d;\n", distribution_item->service, port);
                    }
                    else
                        g_printerr("WARNING: %s is not distributed to any machine. Skipping port assignment...\n");
                }
            }
        }
        
        g_print("  };\n");
        print_port_configuration(&port_configuration);
        g_print("}\n");
        
        destroy_port_configuration(&port_configuration);
        
        if(xml)
        {
            g_free(service_xml);
            g_free(infrastructure_xml);
            g_free(distribution_xml);
            g_free(ports_xml);
        }
        
        return 0;
    }
}
