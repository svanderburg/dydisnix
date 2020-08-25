#ifndef __DYDISNIX_NODE_H
#define __DYDISNIX_NODE_H
#include <nixxml-types.h>
#include <glib.h>

typedef struct Node Node;

struct Node
{
    gchar *name;
    int value;
    NixXML_bool visited;
    void *attachment;
    GPtrArray *links;
};

Node *create_node_with_value(gchar *name, int value);

Node *create_node(gchar *name);

void delete_node(Node *node);

void link_nodes(Node *node_from, Node *node_to);

void unlink_nodes(Node *node_from, Node *node_to);

void link_nodes_bidirectional(Node *node1, Node *node2);

void unlink_nodes_bidirectional(Node *node1, Node *node2);

typedef NixXML_bool (*check_target_node_function) (Node *start_node, Node *examine_node, void *data);

NixXML_bool check_nodes_have_indirect_connection(Node *start_node, Node *examine_node, void *data);

Node *search_node_breadth_first(Node *start_node, check_target_node_function, void *data);

#endif
