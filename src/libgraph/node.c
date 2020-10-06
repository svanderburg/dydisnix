#include "node.h"

Node *create_node_with_value(gchar *name, int value)
{
    Node *node = (Node*)g_malloc(sizeof(Node));
    node->name = name;
    node->value = value;
    node->visited = FALSE;
    node->attachment = NULL;
    node->links = g_ptr_array_new();
    node->link_annotations = g_ptr_array_new();
    return node;
}

Node *create_node(gchar *name)
{
    return create_node_with_value(name, 0);
}

static void delete_node_link_annotations(GPtrArray *link_annotations)
{
    unsigned int i;

    for(i = 0; i < link_annotations->len; i++)
    {
        gchar *annotation = (gchar*)g_ptr_array_index(link_annotations, i);
        g_free(annotation);
    }

    g_ptr_array_free(link_annotations, TRUE);
}

void delete_node(Node *node)
{
    if(node != NULL)
    {
        delete_node_link_annotations(node->link_annotations);
        g_ptr_array_free(node->links, TRUE);
        g_free(node);
    }
}

void link_nodes(Node *node_from, Node *node_to)
{
    g_ptr_array_add(node_from->links, node_to);
}

void link_nodes_with_annotation(Node *node_from, Node *node_to, gchar *annotation)
{
    link_nodes(node_from, node_to);
    g_ptr_array_add(node_from->link_annotations, g_strdup(annotation));
}

void unlink_nodes(Node *node_from, Node *node_to)
{
    unsigned int i;

    for(i = 0; node_from->links->len; i++)
    {
        Node *linked_node = (Node*)g_ptr_array_index(node_from->links, i);

        if(linked_node == node_to)
        {
            g_ptr_array_remove_index(node_from->links, i);

            if(node_from->link_annotations->len > 0) // Also remove link annotation if the table is used
            {
                gchar *annotation = g_ptr_array_index(node_from->link_annotations, i);
                g_ptr_array_remove_index(node_from->link_annotations, i);
                g_free(annotation);
            }

            break;
        }
    }
}

void link_nodes_bidirectional(Node *node1, Node *node2)
{
    link_nodes(node1, node2);
    link_nodes(node2, node1);
}

void link_nodes_bidirectional_with_annotation(Node *node1, Node *node2, gchar *annotation)
{
    link_nodes_with_annotation(node1, node2, annotation);
    link_nodes_with_annotation(node2, node1, annotation);
}

void unlink_nodes_bidirectional(Node *node1, Node *node2)
{
    unlink_nodes(node1, node2);
    unlink_nodes(node2, node1);
}

unsigned int node_degree(const Node *node)
{
    return node->links->len;
}

NixXML_bool check_nodes_have_indirect_connection(Node *start_node, Node *examine_node, void *data)
{
    return (examine_node == (Node*)data);
}

Node *search_node_breadth_first(Node *start_node, check_target_node_function check_target_node, void *data)
{
    GPtrArray *queue = g_ptr_array_new();
    g_ptr_array_add(queue, start_node);
    start_node->visited = TRUE;

    while(queue->len > 0)
    {
        Node *examine_node = g_ptr_array_index(queue, 0);
        g_ptr_array_remove_index(queue, 0);

        if(check_target_node(start_node, examine_node, data))
        {
            g_ptr_array_free(queue, TRUE);
            return examine_node;
        }
        else
        {
            unsigned int i;

            for(i = 0; i < examine_node->links->len; i++)
            {
                Node *adjacent_node = (Node*)g_ptr_array_index(examine_node->links, i);

                if(!adjacent_node->visited)
                {
                    adjacent_node->visited = TRUE;
                    g_ptr_array_add(queue, adjacent_node);
                }
            }
        }
    }

    g_ptr_array_free(queue, TRUE);
    return NULL;
}

void print_node_with_name_dot(FILE *file, const Node *node)
{
    fprintf(file, "\"%s\" [ label = \"%s\" ]\n", node->name, node->name);
}

void print_node_with_name_and_value_dot(FILE *file, const Node *node)
{
    fprintf(file, "\"%s\" [ label = \"<f0> %s|<f1> %d\" ]\n", node->name, node->name, node->value);
}

void print_node_edges_dot(FILE *file, const Node *node)
{
    unsigned int i;

    for(i = 0; i < node->links->len; i++)
    {
        Node *linked_node = (Node*)g_ptr_array_index(node->links, i);

        if(g_strcmp0(node->name, linked_node->name) <= 0) /* Only print an edge from the perspective of the lowest node name. Otherwise, we will get a double connection */
            fprintf(file, "\"%s\" -- \"%s\"\n", node->name, linked_node->name);
    }
}

void print_node_edges_with_annotations_dot(FILE *file, const Node *node)
{
    unsigned int i;

    for(i = 0; i < node->links->len; i++)
    {
        Node *linked_node = (Node*)g_ptr_array_index(node->links, i);
        gchar *annotation = (gchar*)g_ptr_array_index(node->link_annotations, i);

        if(g_strcmp0(node->name, linked_node->name) <= 0) /* Only print an edge from the perspective of the lowest node name. Otherwise, we will get a double connection */
            fprintf(file, "\"%s\" -- \"%s\" [ label = \"%s\", weight=\"%s\" ]\n", node->name, linked_node->name, annotation, annotation);
    }
}
