class DAWGNode:
	def __init__(self, char):
		self.char = char
		self.children = {}
		self.parent = None
		self.final = False

	def get_next(self, char):
		try:
			return self.children[char]
		except KeyError:
			return None

	def set_next(self, char, child):
		self.children[char] = child
		child.parent = self

	def has_transition(self, char):
		return char in self.children


def equivalence(p, q):
	"check if states p and q are equivalent"

	if p.final != q.final:
		return False

	if len(p.children) != len(q.children):
		return False

	s = set(p.children)
	if s != set(q.children):
		return False

	"""
	for c in s:
		if not equivalence(p.children[c], q.children[c]):
			return False
	"""
	for c in s:
		if p.children[c] != q.children[c]:
			return False

	return True


class DAWG:
	def __init__(self):
		self.register = set()
		self.q0 = DAWGNode(None);
		self.wp = ''
		self.nodes = set()

	def add_word(self, word):
		print(self.wp, word)
		# 1. skip existing
		i = 0;
		s = self.q0
		while i < len(word) and s.has_transition(word[i]):
			s = s.get_next(word[i])
			i = i + 1

		assert s != None

		# 2. correct graph
		if i < len(self.wp):
			self.repl_or_reg(s.get_next(self.wp[i]), self.wp[i+1:])


		# 3. add suffix
		while i < len(word):
			n = DAWGNode(word[i])
			self.nodes.add(n)
			s.set_next(word[i], n)
			assert n == s.get_next(word[i])
			s = n
			i = i + 1

		s.final = True
		self.wp = word


	def repl_or_reg(self, state, suffix):
		assert(state != None)
		if suffix:
			node = self.repl_or_reg(state.get_next(suffix[0]), suffix[1:])
			state.set_next(suffix[0], node)

		print("here")
		for r in self.register:
			if equivalence(state, r):
				parent = state.parent
				assert(parent.children[state.char] == state)
				parent.children[state.char] = r

				self.nodes.discard(state)
				return r

		self.register.add(state)
		return state


	def finish(self):
		self.repl_or_reg(self.q0, self.wp)


	def as_dot(self, file):

		def writeln(text=""):
			file.write(text + "\n")

		def aux(node):
			for letter, child in node.children.items():
				writeln("node%d -> node%d [label=\"%c\"]" % (id(node), id(child), letter))
				aux(child)

		writeln("digraph test {")
		for node in self.nodes:
			if node.final:
				writeln('node%d [shape=doublecircle, label=""]' % id(node)) ;
			else:
				writeln('node%d [shape=circle, label=""]' % id(node));

		aux(self.q0)
		writeln("}")


	def words(self):
		L = []
		def aux(node, word):
			if node.final:
				L.append(word)

			for letter, child in node.children.items():
				aux(child, word + letter)

		aux(self.q0, '')
		return L


import os

def main():
	words = "aimaient aimais aimait aime aiment".split()

	n = 1
	def dump():
		nonlocal n
		name = '%d.dot' % n
		with open(name, 'wt') as f:
			D.as_dot(f)

		n += 1
		return name

	D = DAWG()
	for word in words:
		print(word)
		D.add_word(word)
		#dump()

	D.finish()
	name = dump()
	os.system("dotty %s" % name)

	print(words)
	print(D.words())


if __name__ == '__main__':
	main()
