/**
 * @file structof.h
 * @brief Container-of macro for intrusive data structures.
 *
 * Allows recovering a pointer to the enclosing struct from a pointer to
 * one of its members — enabling zero-overhead intrusive lists and trees
 * without extra allocations, as used in lely-core.
 *
 * Naming conventions used in this file:
 * - Macros: SPP_STRUCTOF
 */

#ifndef SPP_STRUCTOF_H
#define SPP_STRUCTOF_H

#include <stddef.h>

/**
 * @brief Recover a pointer to the containing struct from a member pointer.
 *
 * @param p_member  Pointer to the member field.
 * @param type      Type of the containing struct.
 * @param member    Name of the member field inside @p type.
 *
 * @return Pointer to the enclosing @p type instance.
 *
 * Example:
 * @code
 * typedef struct { int x; ListNode_t node; } MyItem_t;
 * ListNode_t *p_node = ...;
 * MyItem_t *p_item = SPP_STRUCTOF(p_node, MyItem_t, node);
 * @endcode
 */
#define SPP_STRUCTOF(p_member, type, member) \
    ((type *)((char *)(p_member) - offsetof(type, member)))

#endif /* SPP_STRUCTOF_H */
