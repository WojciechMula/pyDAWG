import unittest
import pydawg

class TestDAWGBase(unittest.TestCase):
	def setUp(self):
		self.D = pydawg.DAWG();
		if pydawg.unicode:
			self.words = "cat rat attribute tribute war warbute zaaa".split()
		else:
			self.words = b"cat rat attribute tribute war warbute zaaa".split()

	def add_test_words(self):
		for word in sorted(self.words):
			self.assertTrue(self.D.add_word_unchecked(word))

		return self.D


class TestDAWG(TestDAWGBase):
	def test_add_word(self):
		D  = self.D
		if pydawg.unicode:
			w1 = "cat"
		else:
			w1 = b"cat"

		D.add_word(w1)
		D.add_word(w1)	# adding the same word again is ok

		if pydawg.unicode:
			w2 = "catalog"
		else:
			w2 = b"catalog"

		D.add_word(w2)	# ok 'catalog' > 'cat'

		if pydawg.unicode:
			w3 = "any"
		else:
			w3 = b"any"		# failure: 'any' < 'catalog'
		with self.assertRaises(ValueError):
			D.add_word(w3)


	def test_close(self):
		D = self.add_test_words()

		D.close()	# now close
		with self.assertRaises(AttributeError):
			if pydawg.unicode:
				D.add_word("won't work")
			else:
				D.add_word(b"won't work")

		D.clear()	# clear, reset state
		D = self.add_test_words()


	def test_add_word_unchecked(self):
		self.add_test_words()

		if False:
			import dump2dot, os
			with open('1.dot', 'wt') as f:
				dump2dot.dump2dot(D, f)

			os.system("dotty 1.dot")


	def test_len(self):
		D = self.add_test_words()

		self.assertEqual(len(D), len(self.words))
	
	
	def test_clear(self):
		D = self.D

		self.assertEqual(len(D), 0)
		
		D = self.add_test_words()
		self.assertEqual(len(D), len(self.words))

		D.clear()
		self.assertEqual(len(D), 0)


	def test_exists(self):
		D = self.add_test_words()

		for word in self.words:
			self.assertTrue(D.exists(word))
			self.assertTrue(word in D);


		if pydawg.unicode:
			inexisting = "tree horse sky za at".split()
		else:
			inexisting = b"tree horse sky za at".split()

		for word in inexisting:
			self.assertFalse(D.exists(word))
			self.assertTrue(word not in D);


	def test_match(self):
		D = self.add_test_words()
		
		if pydawg.unicode:
			prefixes_true = "c r ra a at attr warb t tr".split()
		else:
			prefixes_true = b"c r ra a at attr warb t tr".split()

		for word in prefixes_true:
			self.assertTrue(D.match(word))
		
		for word in self.words:
			self.assertTrue(D.match(word))
		
		if pydawg.unicode:
			prefixes_false = "hash yellow dark pinapple".split()
		else:
			prefixes_false = b"hash yellow dark pinapple".split()

		for word in prefixes_false:
			self.assertFalse(D.match(word))
		
		if pydawg.unicode:
			self.assertFalse(D.match(""))
		else:
			self.assertFalse(D.match(b""))


	def test_longest_prefix(self):
		D = self.add_test_words()
		
		if pydawg.unicode:
			prefixes = "a at att attri attribu attribut attribute".split()
		else:
			prefixes = b"a at att attri attribu attribut attribute".split()

		for word in prefixes:
			self.assertEqual(D.longest_prefix(word), len(word))

		if pydawg.unicode:
			self.assertEqual(D.longest_prefix("rating"), 3)	# "rat"
			self.assertEqual(D.longest_prefix(""), 0)
			self.assertEqual(D.longest_prefix("y"), 0)
		else:
			self.assertEqual(D.longest_prefix(b"rating"), 3)	# "rat"
			self.assertEqual(D.longest_prefix(b""), 0)
			self.assertEqual(D.longest_prefix(b"y"), 0)

	
	def test_words(self):
		D = self.add_test_words()

		L = D.words()
		self.assertEqual(set(L), set(self.words))


	def test_iter(self):
		D = self.add_test_words()

		L = set(D)
		self.assertEqual(set(L), set(self.words))

	
	def test_iter_invalidate(self):
		D = self.add_test_words()
		
		it = iter(D)
		w = next(it)
		
		D.clear()
		with self.assertRaises(ValueError):
			w = next(it)


	def test_get_stats(self):
		D = self.add_test_words()
		print(self.D.get_stats())
	
	
	def test_get_hash_stats(self):
		D = self.add_test_words()
		print(self.D.get_hash_stats())


class TestDumpLoad(TestDAWGBase):
	def test_dump(self):
		D = self.add_test_words();
		#print(D.bindump())


	def test_load(self):
		D = self.add_test_words()
		L = D.words()
		Ls = D.get_stats()

		dump = D.bindump()
		D.clear()
		D.binload(dump)
		N = D.words()
		Ns = D.get_stats()
		self.assertEqual(L, N)
		self.assertEqual(Ls, Ns)
		
		if pydawg.unicode:
			D.add_word("zip")
			D.add_word("zzza")
		else:
			D.add_word(b"zip")
			D.add_word(b"zzza")
	

	def test_load_empty(self):
		D = self.D
		L = D.words()
		Ls = D.get_stats()

		dump = D.bindump()
		D.binload(dump)
		N = D.words()
		Ns = D.get_stats()
		self.assertEqual(L, N)
		self.assertEqual(Ls, Ns)

		if pydawg.unicode:
			D.add_word("zip")
			D.add_word("zzza")
		else:
			D.add_word(b"zip")
			D.add_word(b"zzza")


class TestPickle(TestDAWGBase):
	def test_pickle_unpickle(self):
		import pickle

		D = self.add_test_words()
		dump = pickle.dumps(D)

		N = pickle.loads(dump)
		self.assertEqual(len(N), len(D))
		self.assertEqual(N.words(), D.words())


class TestMPH(TestDAWGBase):
	def test_word2index(self):
		if pydawg.perfect_hasing:
			D = self.D
			# empty
			for word in self.words:
				index = D.word2index(word)
				self.assertEqual(index, None)

			D = self.add_test_words()

			S = set()
			for word in self.words:
				index = D.word2index(word)
				S.add(index)

			# distinct numbers
			self.assertEqual(len(S), len(D))

			# indexes in range 1..len(D)
			self.assertEqual(min(S), 1)
			self.assertEqual(max(S), len(D))

			# inexising words
			if pydawg.unicode:
				index = D.word2index("xyz")
				self.assertEqual(index, None)
				index = D.word2index("")
				self.assertEqual(index, None)
			else:
				index = D.word2index(b"xyz")
				self.assertEqual(index, None)
				index = D.word2index(b"")
				self.assertEqual(index, None)


	def test_index2word(self):
		if pydawg.perfect_hasing:
			D = self.D

			for i in range(-50, 50):
				word = D.index2word(i)
				self.assertEqual(word, None)
		
			D = self.add_test_words()

			def test(new_words=[]):
				S = set()
				for i in range(1, len(D) + 1):
					word = D.index2word(i)
					S.add(word)

				self.assertEqual(S, set(self.words + new_words))

			# test 1st set
			test()

			if pydawg.unicode:
				word = "zebra"
			else:
				word = b"zebra"

			D.add_word(word)

			# test 2nd set, after adding a word
			test([word])


if __name__ == '__main__':
	unittest.main()
