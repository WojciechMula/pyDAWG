def dumpdata2dot(nodes, edges, file):
	def writeln(text=""):
		file.write(text + "\n")

	writeln("digraph DAWG {");

	for nodeid, end in nodes:
		if end:
			writeln("N%x [shape=doublecircle, label=\"\"]" % nodeid)
		else:
			writeln("N%x [shape=circle, label=\"\"]" % nodeid)

	for srcid, char, dstid in edges:
		if type(char) is bytes:
			char = str(char, "ascii")

		writeln("N%x -> N%x [label=\"%c\"]" % (srcid, dstid, char))

	writeln("}");


def dump2dot(dawg, file):
	nodes, edges = dawg.dump()
	dumpdata2dot(nodes, edges, file)

