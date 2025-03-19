from lxml import etree
import sys

def display_constraint(constraint): 
	print("  ", constraint.tag, end=" ")
	if constraint.tag == "intension": 
		print(constraint.text)
		return

	if constraint.tag == "extension": 
		print(constraint[0].text, end=" ")
		print(constraint[1].tag, constraint[1].text.count('('), "tuples")
		return
	
	if constraint.tag == "regular" or  constraint.tag == "mdd": 
		print(constraint[0].text, end=" ")
		return 
	
	
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
	display_constraint(c)
	
