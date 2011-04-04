import unittest
import pydawg

class TestDAWG(unittest.TestCase):
	def setUp(self):
		self.D = pydawg.DAWG();

	def test_add_word(self):
		words = b"cat rat attribute tribute war warbute zaaa".split()
		for word in sorted(words):
			print(word)
			self.assertTrue(self.D.add_word_unchecked(word))

		if False:
			import dump2dot, os
			with open('1.dot', 'wt') as f:
				dump2dot.dump2dot(self.D, f)

			os.system("dotty 1.dot")


	def test_exists(self):
		words = b"cat rat attribute tribute war warbute zaaa".split()
		for word in sorted(words):
			self.assertTrue(self.D.add_word_unchecked(word))

		for word in words:
			self.assertTrue(self.D.exists(word))
			self.assertTrue(word in self.D);


		inexisting = b"tree horse sky za at".split()
		for word in inexisting:
			self.assertFalse(self.D.exists(word))
			self.assertTrue(word not in self.D);


	def test_match(self):
		words = b"cat rat attribute tribute war warbute zaaa".split()
		for word in sorted(words):
			self.assertTrue(self.D.add_word_unchecked(word))

		prefixes_true = b"c r ra a at attr warb t tr".split()
		for word in prefixes_true:
			self.assertTrue(self.D.match(word))
		
		for word in words:
			self.assertTrue(self.D.match(word))
		
		prefixes_false = b"hash yellow dark pinapple".split()
		for word in prefixes_false:
			self.assertFalse(self.D.match(word))
		
		self.assertFalse(self.D.match(b""))


	def test_longest_prefix(self):
		words = b"cat rat attribute tribute war warbute zaaa".split()
		for word in sorted(words):
			self.assertTrue(self.D.add_word_unchecked(word))

		prefixes = b"a at att attri attribu attribut attribute".split()
		for word in prefixes:
			self.assertEqual(self.D.longest_prefix(word), len(word))

		self.assertEqual(self.D.longest_prefix(b"rating"), 3)
		self.assertEqual(self.D.longest_prefix(b""), 0)
		self.assertEqual(self.D.longest_prefix(b"y"), 0)

	
	def test_words(self):
		words = b"cat rat attribute tribute war warbute zaaa".split()
		for word in sorted(words):
			self.assertTrue(self.D.add_word_unchecked(word))

		L = self.D.words()
		self.assertEqual(set(L), set(words))


	def test_get_stats(self):
		words = b"cat rat attribute tribute war warbute zaaa".split()
		for word in sorted(words):
			self.assertTrue(self.D.add_word_unchecked(word))

		#print(self.D.get_stats())


if __name__ == '__main__':
	unittest.main()
