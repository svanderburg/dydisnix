#include "node.h"

Node *create_node_with_value(gchar *name, int value)
{
    Node *node = (Node*)g_malloc(sizeof(Node));
    node->name = name;
    node->value = value;
    node->visited = FALSE;
    node->attachment = NULL;
    node->links = g_ptr_array_new();
    return node;
}

Node *create_node(gchar *name)
{
    return create_node_with_value(name, 0);
}

void delete_node(Node *node)
{
    if(node != NULL)
    {
        g_ptr_array_free(node->links, TRUE);
        g_free(node);
    }
}

void link_nodes(Node *node_from, Node *node_to)
{
    g_ptr_array_add(node_from->links, node_to);
}

void unlink_nodes(Node *node_from, Node *node_to)
{
    unsigned int i = 0;

    for(i = 0; node_from->links->len; i++)
    {
        Node *linked_node = (Node*)g_ptr_array_index(node_from->links, i);

        if(linked_node == node_to)
        {
            g_ptr_array_remove_index(node_from->links, i);
            break;
        }
    }
}

void link_nodes_bidirectional(Node *node1, Node *node2)
{
    link_nodes(node1, node2);
    link_nodes(node2, node1);
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
