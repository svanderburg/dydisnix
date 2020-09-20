#ifndef __DYDISNIX_LASTASSIGNMENTSTABLE_H
#define __DYDISNIX_LASTASSIGNMENTSTABLE_H
#include <stdio.h>
#include <libxml/parser.h>
#include <glib.h>

GHashTable *create_empty_last_assignments_table(void);

void *parse_last_assignments_table(xmlNodePtr element, void *userdata);

void delete_last_assignments_table(GHashTable *last_assignments_table);

void print_last_assignments_table_nix(FILE *file, GHashTable *last_assignments_table, const int indent_level, void *userdata);

void print_last_assignments_table_xml(FILE *file, GHashTable *last_assignments_table, const int indent_level, const char *type_property_name, void *userdata);

void remove_invalid_last_assignments(GHashTable *last_assignments_table, GHashTable *id_resources_table);

#endif
