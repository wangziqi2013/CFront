#
# common.py - Debugging procedures
#

import os
import sys
import inspect

#####################################################################
# The following are debugging procedures
#####################################################################

# Turn this to False to disable debugging output
# The derived class could set or unset this flag to disable
# debug printing
#
# In order to assign to this variable, do the following in the
# derived class file:
#
#   import analysis_common
#   ...
#   analysis_common.debug_flag = False
#
# And after the assignment all debug printing will be disabled
debug_flag = True

def print_test_name():
    """
    Prints the current function name as test name using special format

    :return: None
    """
    parent_name = inspect.currentframe().f_back.f_code.co_name

    dbg_printf("==============================")
    dbg_printf(parent_name)
    dbg_printf("==============================")

    return

def dbg_printf(format, *args, **kwargs):
    """
    C-Style function that writes debugging output to the terminal

    The keyword argument are specified as follows:
       "no_newline": if set to True then do not print new line
       By default a new line will be printed

       "flush": if set to True then sys.stderr will be flushed
       before this function returns. If '\r' is used instead
       of '\n' then we need to flush it every time because
       the carriage return will not cause a flush

       "no_header": If set then the header will not be printed

    :param format: The format string
    :param args: Arguments
    :return: None
    """
    # Do not print anything annoying
    if debug_flag is False:
        return

    # If the header is required then print the header
    if not (kwargs.get("no_header", None) is True):
        frame = inspect.currentframe()
        prev_frame = frame.f_back
        code = prev_frame.f_code
        prev_name = code.co_name

        # Make it more human readable by replacing the name with
        # an easy to understand one
        if prev_name == "<module>":
            prev_name = "[Top Level Module]"
        else:
            prev_name += "()"

        # Write the prologue of debugging information
        sys.stderr.write("%-28s: " % (prev_name,))

    format = format % tuple(args)
    sys.stderr.write(format)

    # If we did not specify no_newline then print a
    # new line. Otherwise do not use new line
    if not (kwargs.get("no_newline", None) is True):
        # So we do not need to worry about new lines
        sys.stderr.write('\n')

    # If flush is required then just flush
    if kwargs.get("flush", None) is True:
        sys.stderr.flush()

    return


#####################################################################
# Argument Handling
#####################################################################

class Argv:
    """
    Helps with argument vector processing
    """
    def __init__(self, argv=sys.argv[1:]):
        """
        This function processes argv and decomposes into two parts:
          (1) Key-value pairs, specified by -key=val or --key=val
              For non value-carrying flags the value is None
          (2) Argument lists, i.e. those that are not key value pairs
              They appear in the same order as in the original command
              line
          (3) Everything after -- is considered as arguments
        """
        # This is the key value dict with value optionally being present
        self.key_value_dict = {}
        # This is the list of arguments which
        self.arg_list = []

        # If this is true then we do not distinguish between arguments and
        # key value pairs and just push everything
        push_all = False
        for arg in argv:
            # If we have seen '--' in the input stream then we push everything
            # remains in the argv
            if arg == '--':
                push_all = True
                continue

            # If we have seen '--' then just push it back and loop
            if push_all is True:
                self.arg_list.append(arg)
                continue

            # Otherwise check whether it is a kv pair
            is_kv_pair = False
            if arg.startswith('--'):
                is_kv_pair = True
                arg = arg[2:]
            elif arg.startswith('-'):
                is_kv_pair = True
                arg = arg[1:]

            # if it is a kv pair then we process
            if is_kv_pair is True:
                # Only split the leftmost occurrence of '=' if there is one
                kv_pair = arg.split('=', 1)

                # We do allow duplicated flags, and prepares a list for it
                if self.key_value_dict.has_key(kv_pair[0]) is False:
                    self.key_value_dict[kv_pair[0]] = []

                if len(kv_pair) == 1:
                    # If there is no equality sign then we know there is no
                    # value and only a single flag
                    self.key_value_dict[kv_pair[0]].append(None)
                else:
                    # Otherwise make a key value mapping
                    self.key_value_dict[kv_pair[0]].append(kv_pair[1])
            else:
                self.arg_list.append(arg)

        return

    def has_key(self, key):
        """
        Whether a flag exists

        :param key: The flag name, without - or --
        :return: bool
        """
        return self.key_value_dict.has_key(key)

    def has_keys(self, *args):
        """
        Whether any of the key in the args list exists

        :param args: A list of keys
        :return: bool
        """
        for key in args:
            if self.has_key(key) is True:
                return True

        return False

    def get_value(self, key):
        """
        This function returns the value list for key specified in the
        argument

        :param key: The flag name
        :return: list of objects or None if not found
        """
        return self.key_value_dict.get(key)

    def get_all_values(self, *args):
        """
        This function returns all values associated with all keys in the
        variable length arguments. The result is a concatenated list
        of all values with all keys.

        If one or more of the keys do not exist we simply ignore them

        :param args: A list of keys
        :return: A list of value strings and possibly empty list
        """
        ret_list = []
        for key in args:
            ret = self.get_value(key)
            if ret is not None:
                ret_list += ret

        return ret_list

#####################################################################
# Unit Test Framework (Dependency Testing)
#####################################################################

# This is the name of the dependency set
DEP_SET_NAME = "dep_set"
DEP_SET_COPY_NAME = "dep_set_copy"
# If this is set to True then the function has already
# been tested. If it is False then we know the function
# is a testing function and is waiting to be run
TEST_FINISH_FLAG_NAME = "test_finish_flag"

class TestNode:
    """
    This class is used as a function decorator with parameters.

    It adds attributes to function objects with a flag indicating
    testing functions, and also with a dependency set which
    represents tests that must be run prior to the decorated one

    The usage of this class as function decorator is like this:

        (1) Decorating instance functions

        # Use string name of dependencies
        @TestNode("dep1", "dep2", "dep3")
        def test_some_feature(self, *args):
            ...
            return

        (2) Decorating static functions

        # Note that we must put @staticmethod on top of this
        # decorator because static method will add another level
        # of wrapper
        @staticmethod
        @TestNode("dep1, dep2", "dep3")
        def test_static_function(*args):
            ...
            return

    Note that the name of the function being decorated must begin
    with "test_" to distinguish it from others
    """
    def __init__(self, *args):
        """
        Initialize the instance with a callable object and a
        list of dependencies

        :param dep_set: A set of dependencies
         (i.e. function name)
        """
        # Note that we do not unpack args since set() requires
        # a list as the argument
        self.dep_set = set(args)

        return

    # This is the error message when an attribute we would like
    # to add already exist in the function's attribute entries
    ATTRIBUTE_ALREADY_EXIST_ERROR = \
        "Attribute %s is already present"

    def __call__(self, func):
        """
        Call back function when the object is called like
        a function

        This makes the object a callable object

        :param func: The function object being decorated
        :return: What the function returns
        """
        # First check whether the dep_set has already been set
        # if it is present then just raise an error
        if hasattr(func, DEP_SET_NAME) is True:
            raise AttributeError(self.ATTRIBUTE_ALREADY_EXIST_ERROR %
                                 (DEP_SET_NAME, ))

        if hasattr(func, TEST_FINISH_FLAG_NAME) is True:
            raise AttributeError(self.ATTRIBUTE_ALREADY_EXIST_ERROR %
                                 (TEST_FINISH_FLAG_NAME, ))

        # Otherwise just associate the function with the dependency
        # set we initialize this class instance with
        setattr(func, DEP_SET_NAME, self.dep_set)

        # Also set the testing finished flag to False to indicate:
        #   (1) The function is a testing function
        #   (2) The function is ready to be run
        setattr(func, TEST_FINISH_FLAG_NAME, False)

        # We do not return a wrapper as normally most decorators
        # do. Instead we return the original function with a little
        # bit more
        return func

class DebugRunTestCaseBase:
    """
    This class serves as the base class of all test cases that require
    certain dependencies to run.

    For example, if there are three cases, test_1, test_2 and test_3,
    where test_2 should be run after test_1 and test_3, then in the normal
    configuration, when we try to enumerate all functions beginning with
    "test_", the order returned by the interpret is undefined. Therefore
    we associate dependency information with test cases, and do a topological
    sorting in the runtime. Those with zero dependencies are executed first,
    and then removed from the dependency list of other test cases.

    To use this class, please read the following instructions:

      (1) All test cases should begin with "test_" in order to be recognized
          as a testing function
      (2) In order to establish dependencies please use the decorator
          @TestNode(dep1, dep2, ...) where dep1, dep2, ... are string names
          of testing functions that should be executed before it. The dependency
          relation is transitive, so if dep2 depends on dep1 and dep3 depends
          on dep2, we could just write @TestNode("dep2") on dep3 and
          @TestNode("dep1") on dep2.
      (3) If the testing function is also a static function, please put
          @staticmethod decorator above @TestNode, because @staticmethod
          will add another level of wrapper class which does not play
          well with @TestNode()
      (4) If there is a dependency loop, then none of those testing functions
          in the loop will be executed. This will be detected by the
          verification function at the end of an entire run, and an error
          will be thrown if some testing functions do not get executed
      (5) The same instance could be reused, i.e. the dependency list
          and execution status will be restored after all test cases have
          been run successfully. However in case of an error this is not
          guaranteed to happen
    """

    COULD_NOT_FIND_ATTRIBUTE_ERROR = "Could not find attribute: %s"

    def __init__(self):
        """
        Empty initializer. All initialization should be done in
        the derived class
        """
        return

    @staticmethod
    def is_instance_method(func):
        """
        Check whether a callable object is an instance method by
        checking its __self__ and __func__ attributes

        Also this function always returns False for objects without
        a __call__ method

        :param func: Any object
        :return: boolean
        """
        # Must check whether it is a callable object
        if hasattr(func, "__call__") is False:
            return False

        return hasattr(func, "__func__") and \
               hasattr(func, "__self__")

    def backup_settings(self):
        """
        Backup all settings before running the test case because
        the dependency set will be altered during the testing

        :return: None
        """
        for func_name in dir(self):
            if func_name.startswith("test_") is False:
                continue

            func = getattr(self, func_name, None)
            assert (func is not None)

            if DebugRunTestCaseBase.is_instance_method(func) is True:
                func = getattr(func, "__func__", None)
                assert (func is not None)

            if hasattr(func, TEST_FINISH_FLAG_NAME) is False:
                continue
            elif hasattr(func, DEP_SET_NAME) is False:
                continue

            dep_set = getattr(func, DEP_SET_NAME, None)
            assert(dep_set is not None)

            # Make a shallow copy of the set. Since its members are strings
            # shallow copy does not matter
            dep_set_copy = dep_set.copy()

            # And then make it an attribute of the underlying
            # function no matter whether it is static or instance
            setattr(func, DEP_SET_COPY_NAME, dep_set_copy)

        return

    def restore_settings(self):
        """
        This function restores all settings that are backup up
        before the testing is being run. This prepares a testing
        instance that could be run again

        :return: None
        """
        for func_name in dir(self):
            if func_name.startswith("test_") is False:
                continue

            func = getattr(self, func_name, None)
            assert (func is not None)

            if DebugRunTestCaseBase.is_instance_method(func) is True:
                func = getattr(func, "__func__", None)
                assert (func is not None)

            if hasattr(func, TEST_FINISH_FLAG_NAME) is False:
                continue
            elif hasattr(func, DEP_SET_NAME) is False:
                continue

            # Retrieve the copied version. Since it is a duplication
            # rather than reference to the existing dep set, we
            # could directly set it as the current dep set and
            # just discard the dep set. The next round of test case
            # run will duplicate it agian
            dep_set_copy = getattr(func, DEP_SET_COPY_NAME, None)
            assert(dep_set_copy is not None)

            # Set the attribute directly by replacing the previous one
            setattr(func, DEP_SET_NAME, dep_set_copy)

            # And then remove the copy to avoid further problem
            delattr(func, DEP_SET_COPY_NAME)

            # At last also need to reset the test finished flag to False
            setattr(func, TEST_FINISH_FLAG_NAME, False)

        return

    def verify_test_run(self):
        """
        Verify that all tests have been run by checking whether
        all flags are False and whether all dependency sets are
        empty. If not raise error

        :return: None
        """
        dbg_printf("Verifying that all tests complete properly...")

        for func_name in dir(self):
            # Do not process thoese that do not start with "test_"
            if func_name.startswith("test_") is False:
                continue

            func = getattr(self, func_name, None)
            assert(func is not None)

            # For instance methods we need to get one more level
            # into the object because the real function object is
            # wrapped
            if DebugRunTestCaseBase.is_instance_method(func) is True:
                func = getattr(func, "__func__", None)
                # We know this is true because we use __func__ to
                # check whether it is an instance method
                assert(func is not None)

            # Skip those that do not have required attributes
            if hasattr(func, TEST_FINISH_FLAG_NAME) is False:
                continue
            elif hasattr(func, DEP_SET_NAME) is False:
                continue

            flag = getattr(func, TEST_FINISH_FLAG_NAME, None)
            assert(flag is not None)
            if flag is False:
                raise RuntimeError("Function %s is not run during testing" %
                                   (func_name, ))

            dep_set = getattr(func, DEP_SET_NAME, None)
            assert(dep_set is not None)
            if len(dep_set) != 0:
                raise RuntimeError("Dependency set for function %s" + \
                                   " is not empty!" %
                                   (func_name, ))

        return

    def choose_next_test(self):
        """
        This function builds a dependency graph by performing topological
        sort on nodes. It chooses the next test function to run by selecting
        those functions that satisfy the following properties:

        :return: A pair attr_name, func
                 The second element is a function object, either bound or
                 not bounded. The first element is the name of the function

                 None if all tests are run
        """
        for attr_name in dir(self):
            # Skip those that are not test_
            if attr_name.startswith("test_") is not True:
                continue

            # This is the function object we are working on
            func = getattr(self, attr_name, None)
            assert(func is not None)

            # Check whether the test has been finished, or whether
            # this is a testing function. If not then skip them
            test_finished_flag = getattr(func, TEST_FINISH_FLAG_NAME, None)
            if test_finished_flag is None:
                continue
            elif test_finished_flag is True:
                continue

            # Then get its dependency set, and we need to choose the one
            # with an empty dep set
            dep_set = getattr(func, DEP_SET_NAME, None)
            if dep_set is None:
                raise AttributeError(self.COULD_NOT_FIND_ATTRIBUTE_ERROR %
                                     (DEP_SET_NAME, ))

            # Choose the one that has no dependency
            if len(dep_set) == 0:
                return attr_name, func

        return "[end of test]", None

    def finish_test(self, func_name):
        """
        This function marks the function as finished, and also remove
        it from all dependants

        :param func_name: The name of the function
        :return: None
        """
        # First check whether there is such function object
        if hasattr(self, func_name) is False:
            raise AttributeError(self.COULD_NOT_FIND_ATTRIBUTE_ERROR %
                                 (func_name, ))

        # This might be an instance method since we use instance as
        # argument to getattr(), so if this is an instance method
        # we need to resolve its underlying function
        func = getattr(self, func_name)
        # For bound methods, __func__ is the unbounded function
        # and __self__ is the reference to the instance object
        if DebugRunTestCaseBase.is_instance_method(func) is True:
            func = getattr(func, "__func__", None)
            assert(func is not None)

        # Then check whether the function has two attributes
        if hasattr(func, TEST_FINISH_FLAG_NAME) is False:
            raise AttributeError(self.COULD_NOT_FIND_ATTRIBUTE_ERROR %
                                 (TEST_FINISH_FLAG_NAME, ))
        elif hasattr(func, DEP_SET_NAME) is False:
            raise AttributeError(self.COULD_NOT_FIND_ATTRIBUTE_ERROR %
                                 (DEP_SET_NAME, ))

        # Set the test finished flag to True
        setattr(func, TEST_FINISH_FLAG_NAME, True)

        # Next for all testing function that has not been completed
        # remove the function just finished from its dependency set
        for attr in dir(self):
            if attr.startswith("test_") is False:
                continue

            func = getattr(self, attr)
            if hasattr(func, TEST_FINISH_FLAG_NAME) is False:
                continue
            elif hasattr(func, DEP_SET_NAME) is False:
                continue
            elif getattr(func, TEST_FINISH_FLAG_NAME) is True:
                # Also filter out those who has finished
                continue

            # Then get dependency set
            dep_set = getattr(func, DEP_SET_NAME, None)
            assert(dep_set is not None)

            # Remove the function name as a dependency in
            # the set
            if func_name in dep_set:
                dep_set.remove(func_name)

        return

    def run_tests(self, argv):
        """
        Run all tests with prefix "test_"

        :return: None
        """
        # First backup the dep sets
        self.backup_settings()

        func_name, func = self.choose_next_test()
        while func is not None:
            # Run the test case
            func(argv)
            self.finish_test(func_name)

            func_name, func = self.choose_next_test()

        # This checks whether all testing functions that are
        # decorated have been run properly
        # Also note that this must happen before tests are run
        self.verify_test_run()

        # And then restore the dep set after all test cases
        # have been run
        self.restore_settings()

        return