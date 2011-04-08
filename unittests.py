import unittest
import pydawg

class TestDAWGBase(unittest.TestCase):
	def setUp(self):
		self.D = pydawg.DAWG();
		self.words = b"cat rat attribute tribute war warbute zaaa".split()

	def add_test_words(self):
		for word in sorted(self.words):
			self.assertTrue(self.D.add_word_unchecked(word))

		return self.D


class TestDAWG(TestDAWGBase):
	def test_add_word(self):
		D  = self.D
		w1 = b"cat"
		D.add_word(w1)
		D.add_word(w1)	# adding same word again is ok

		w2 = b"catalog"
		D.add_word(w2)	# ok 'catalog' > 'cat'

		w3 = b"any"		# failure: 'any' < 'catalog'
		with self.assertRaises(ValueError):
			D.add_word(w3)


	def test_close(self):
		D = self.add_test_words()

		D.close()	# now close
		with self.assertRaises(AttributeError):
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


		inexisting = b"tree horse sky za at".split()
		for word in inexisting:
			self.assertFalse(D.exists(word))
			self.assertTrue(word not in D);


	def test_match(self):
		D = self.add_test_words()

		prefixes_true = b"c r ra a at attr warb t tr".split()
		for word in prefixes_true:
			self.assertTrue(D.match(word))
		
		for word in self.words:
			self.assertTrue(D.match(word))
		
		prefixes_false = b"hash yellow dark pinapple".split()
		for word in prefixes_false:
			self.assertFalse(D.match(word))
		
		self.assertFalse(D.match(b""))


	def test_longest_prefix(self):
		D = self.add_test_words()

		prefixes = b"a at att attri attribu attribut attribute".split()
		for word in prefixes:
			self.assertEqual(D.longest_prefix(word), len(word))

		self.assertEqual(D.longest_prefix(b"rating"), 3)
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


if __name__ == '__main__':
	unittest.main()
