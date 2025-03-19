from lxml import etree
import sys
import re


def simp(str):
    return re.sub(' +', ' ', str.replace('\n', ' | ').strip())


def display_constraint(constraint):
    print("  ", constraint.tag, end=" ")
    if constraint.tag == "intension":
        print(simp(constraint.text))
        return

    if constraint.tag == "extension":
        print(simp(constraint[0].text), end=" ")
        print(constraint[1].tag, constraint[1].text.count('('), "tuples")
        return

    if constraint.tag == "regular" or constraint.tag == "mdd":
        print(simp(constraint[0].text))
        return

    if constraint.tag == "allDifferent":
        if len(constraint) == 0:
            print(simp(constraint.text))
            return
        if constraint[0].tag == "matrix":
            print("matrix", simp(constraint[0].text))
            return
        if len(constraint) == 1:
            print(simp(constraint[0].text))
            return

        if len(constraint) == 2:
            if constraints[1].tag == "except":
                print("except ", simp(constraint[0].text))
                return
        print("list")
        return
    if constraint.tag == "allEqual":
        if len(constraint) == 0:
            print(simp(constraint.text))
        else:
            print(simp(constraint[0].text))
        return

    if constraint.tag == "ordered":
        if len(constraint) == 0:
            print(constraint.attrib, simp(constraint.text))
            return
        if len(constraint) == 2:
            print(simp(constraint[1].text), simp(constraint[0].text))
            return
        print(simp(constraint[2].text), simp(constraint[0].text), simp(constraint[1].text))

    if constraint.tag == "lex":
        if constraint[0].tag == "list":
            print(" list (", len(constraint) - 1, ")", constraint[-1].text.strip())
            return
        print("matrix", simp(constraint[-1].text))
        return

    if constraint.tag == "sum":
        print(simp(constraint[0].text), simp(constraint[-1].text))
        return

    if constraint.tag == "count":
        print(simp(constraint[0].text), " v:", simp(constraint[1].text), simp(constraint[2].text))
        return

    if constraint.tag == "nValues" or constraint.tag == "minimum" or constraint.tag == "maximum" or constraint.tag == "channel":
        print(simp(constraint[0].text), simp(constraint[-1].text))
        return

    if constraint.tag == "cardinality":
        print(simp(constraint[0].text), " v:", simp(constraint[1].text), " o:", simp(constraint[2].text))
        return

    if constraint.tag == "element":
        if len(constraint) == 3:
            print(simp(constraint[0].text), " i:", simp(constraint[1].text), " v:", simp(constraint[2].text))
            return
        else:
            print(simp(constraint[0].text), simp(constraint[1].text))
            return

    if constraint.tag == "cardinality":
        print(simp(constraint[0].text), " v:", simp(constraint[1].text), " o:", simp(constraint[2].text))
        return

    if constraint.tag == "stretch":
        print(simp(constraint[0].text), " v:", simp(constraint[1].text), " w:", simp(constraint[2].text))
        return

    if constraint.tag == "noOverlap":
        print(simp(constraint[0].text), " l:", simp(constraint[1].text))
        return

    print()


def block(constraint):
    if constraint.tag == "group":
        print("   group", end=" ")
        display_constraint(constraint[0])
        print("           ", len(constraint) - 1, "args,  first:", simp(constraint[1].text))
        print()
        return

    if constraint.tag == "block":
        for c2 in constraint:
            block(c2)
        return
    display_constraint(constraint)


print()

tree = etree.parse(sys.argv[1])

print("Bench :", sys.argv[1])

print()

print("Variables: ")
variables = tree.xpath("/instance/variables/*")

for v in variables:
    print(" ", v.tag, v.attrib, v.text)

print("\nConstraints: ")

constraints = tree.xpath("/instance/constraints/*")

for c in constraints:
    block(c)

objective = tree.xpath("/instance/objectives/*")
if len(objective) > 0:
    print("\nObjective:")
    o = objective[0]
    print("   ", o.attrib, end=" ")
    if len(o) == 0:
        print("   ", simp(o.text))
    else:
        print("   ", simp(o[0].text))

print("\n")
