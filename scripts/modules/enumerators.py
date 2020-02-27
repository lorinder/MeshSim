#!/usr/bin/env python

"""
Small module to enumerate sets of variable values.

For simulation, we often want the enumerate the possible values in a
grid, i.e. look at the cartesian product possible values.  Sometimes the
set is slightly more complicated, but usually not much so.  In such
cases, the explicit listing of parameter values together with the
cartesian_ext enumerator, as described below, comes in handy.

Enumerators are implemented as python generators that generate the
parameter sets sequentially.

The following types of enumerators are available:
    * cartesian
    * cartesian_ext
    * union
    * list
    * singleton

For a brief explanation of the enumerators, read up the documentation of
the corresponding enum_* functions below.

The get_enumerator_from_dict() is 
"""

import sys

def enum_cartesian(dict_vars):
    """Create a cartesian enumerator from a dict of vars

    Takes a dict of the form

        { 'var_a': [ list of var_a values ],
          'var_b': [ list of var_b values ],
          ...
        }

    The cartesian() generator then emits dicts with all the possible
    combinations; e.g. if var_a can have values 1, 2, 3, and var_b can have
    values 'a', 'b', there will be six pairs.
    """

    if len(dict_vars) == 0:
        return
    if len(dict_vars) == 1:
        key = next(iter(dict_vars.keys()))
        for val in dict_vars[key]:
            yield { key: val }
        return

    # General case
    key = next(iter(dict_vars.keys()))
    dict2 = {}
    for k, v in dict_vars.items():
        if k == key:
            continue
        dict2[k] = v
    for v in dict_vars[key]:
        for d in enum_cartesian(dict2):
            d[key] = v
            yield d

def enum_cartesian_ext(enums):
    """Create a cartesian_ext enumerator from a list of enumerators.

    A cartesian_ext enumerator takes N enumerators; each of those
    enumerators is assumed to enumerate disjoint parameter sets.  For
    example one enumerator could enumerate all the dicts with values for
    parameters a and b, and another one could list all the dicts with
    values for parameters c and d.  Then every combination of values for
    a and b is joined with every combination of values for c and d.
    """
    if len(enums) == 0:
        return
    E = enums[-1]
    if len(enums) == 1:
        for d in E:
            yield d
        return

    # General case:
    E = list(E)
    for d1 in enum_cartesian_ext(enums[:-1]):
        for d2 in E:
            # Need to copy d2 here, since otherwise for each iteration
            # in the outer loop (d1), the same dicts are reused; causing
            # previous enumerations to be overwritten.
            d2 = dict(d2)
            d2.update(d1)
            yield d2

def enum_union(enums):
    """Produce a union enumerator from a python list of enumerators.

    This takes a list of enumerators, and first enumerates all the
    parameter sets in the first enumerator, followed by those in the
    second enumerator, etc.  In other words, it produces the union of
    all the parameter sets.  (Duplicates are not removed.)
    """

    for e in enums:
        for d in e:
            yield d

def enum_list(list_of_dicts):
    """Produce a list enumerator from a python list.

    The list enumerator returns each item in the explicitly provided
    list, one after another.  In other words, The input is a list of
    dicts (the dicts containing parameter->value assignments), and one
    of those dicts will be yielded by each iteration.
    """
    for d in list_of_dicts:
        yield d

def enum_singleton(vars_dict):
    """Produce a singleton enumerator.

    The singleton enumerator produces just a single set of values: the
    one provided.  This is syntactic sugar, the same can be achieved
    with a list enumerator or a cartesian enumerator, but in each case
    with a bit more baggage.
    """
    yield vars_dict

###

def get_enumerator_from_dict(d):
    """Generically produce an enumerator from a python dict.

    This is typically used after deserializing JSON into python objects
    to get the actual enumerator.

    cartesian enumerator syntax:
      { "type": "cartesian", "vars": { "var_a": [ ... ], ... } }

    cartesian_ext enumerator syntax:
      { "type": "cartesian_ext", "enums": [ { ... }, { ... }, ... ]

    union enumerator syntax:
      { "type": "union", "enums": [ { ... }, { ... }, ... ]

    list enumerator syntax:
      { "type": "list", "list": [ { ... }, ... ] }

    singleton enumerator syntax:
      { "type": "singleton", "vars": { "var_a": value_a, ... } }
    """
    assert type(d) == dict;
    tp = d["type"]
    if tp == "cartesian":
        return enum_cartesian(d["vars"])
    elif tp == "cartesian_ext":
        enums = [ get_enumerator_from_dict(x) for x in d["enums"] ]
        return enum_cartesian_ext(enums)
    elif tp == "union":
        enums = [ get_enumerator_from_dict(x) for x in d["enums"] ]
        return enum_union(enums)
    elif tp == "list":
        return enum_list(d["list"])
    elif tp == "singleton":
        return enum_singleton(d["vars"])
    sys.stderr.write("Error:  Not a known enumerator type: `%s'\n" % tp)
    return None

def _usage():
    print("""Parameter set enumerator.

    usage:  enumerators.py [-h] <json-string-or-filename>

    enumerators.py is typically used as a python module in other tools
    (e.g., stagesim) to create enumerators from dicts.  But It can be
    run on the command line for testing purposes.  With no options
    given, and just a JSON string or the name of a file containing JSON,
    it will produce the enumeration, and list all the combinations, one
    per line, on the command line.

    Options:
        -h      print this help and exit

        -t      run tests and exit
    """)
    sys.exit(0)

def _tests():
    ex_r_pairs = [ ("""
    {
      "type": "cartesian_ext",
      "enums": [
        { "type": "singleton",
          "vars": {
          }
        },
        { "type": "cartesian_ext",
          "enums": [
            { "type": "cartesian",
              "vars": {
                "a": [ 1, 2, 3, 4 ]
              }
            },
            { "type": "singleton",
              "vars": {
              }
            }
          ]
        }
      ]
    }""", [ {'a': 1}, {'a': 2}, {'a': 3}, {'a': 4} ]) ]
    for example, result in ex_r_pairs:
        d = json.loads(example)
        r_received = list(get_enumerator_from_dict(d))
        if r_received != result:
            print("Failure in example:", example)
            print("Expected:", result)
            print("Got:", r_received)
            sys.exit(1)
    print("All tests passed OK.")
    sys.exit(0)

if __name__ == "__main__":
    import json
    import getopt

    opts, args = getopt.getopt(sys.argv[1:], "ht")
    for o, a in opts:
        if o == '-h':
            _usage()
        elif o == '-t':
            _tests()

    if len(args) != 1:
        sys.stderr.write("Error:  Need exactly one argument on command "
          + "line:  description string or filename.\n")
        sys.exit(1)
    arg = args[0]

    # load JSON
    if arg[0] == '{':
        descr = json.loads(arg)
    else:
        descr = json.load(open(arg, 'r'))

    # Get the combinations
    for d in get_enumerator_from_dict(descr):
        s = ""
        for k in sorted(d.keys()):
            s += "%s=%s " % (k, d[k])
        print(s)
