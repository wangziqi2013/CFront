#
# This file includes routines for transforming the abstract
# syntax tree
#

def transform_type_decl(root):
    """
    This function transforms the ABS

    :param root: The T_DECL node
    :return: SyntaxNode, bool
    """
    return root, False

# This is the dict for transforming the AST
# The key is the type of the syntax node, and the value
# is the routine for transforming it.
#
# The return value of the function should be also a
# syntax node object, and it will be used to replace
# the node we passed to the function
TRANSFORM_DICT = {
    "T_DECL": transform_type_decl,
}

def transform_ast(root):
    """
    This function traverses the AST using pre-order traversal
    and then invoke routines to transform nodes based on its
    node type

    Note that in case we exceed the maximum depth for recursion
    in a very deep AST, we maintain a stack manually in this function
    and emulate recursion using the stack

    The return value also indicates whether we need to transform
    the child node of the current node after it has been transformed.
    If the returned boolean is False then we do not attempt to
    transform its child nodes; Otherwise we continue with its child

    :param root: The root of the AST
    :return: root (may have been changed), bool
    """
    # This stores tuple: (child list, index)
    stack = []

    # To keep consistency we pretend that the root also comes
    # from a child list of only one element that is the root
    root_child_list = [root]

    current_child_list = root_child_list
    current_index = 0
    current_level = 0

    while True:
        # While there is still a node to transform in the child list
        while current_index < len(current_child_list):
            current_node = current_child_list[current_index]
            func = TRANSFORM_DICT.get(current_node.symbol, None)

            # Enable this to check we are doing it correctly
            #print " " * current_level + current_node.symbol

            # If the current node has something to transform
            if func is not None:
                new_node, transform_child = func(current_node)
                # Update the node into the child list
                # Note that the child list is just a reference
                # so we could update it directly and the change will be
                # reflected into the syntax node
                current_child_list[current_index] = new_node
            else:
                # Otherwise we must continue transforming the children
                # of the current node
                transform_child = True

            # If transform child is True then just append the current
            # index into the stack and start a new instance
            if transform_child is True:
                stack.append((current_child_list, current_index + 1))
                current_child_list = current_node.child_list
                current_index = 0
                current_level += 1
                continue
            else:
                current_index += 1

        # If the stack is empty which means we have finished transforming all
        # nodes, then just return the new root node
        if len(stack) == 0:
            break
        else:
            # Just finished the current node's children, need to
            # go up one level and continue
            current_child_list, current_index = stack.pop()
            current_level -= 1

    return root_child_list[0]
