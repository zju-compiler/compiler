#include "symtab.h"
#include <fstream>
using namespace std;

//// type name begin with '0' means it is only a reference
void Symtab::printSymtab(ofstream &out) {
	cout << "in printSymtab" << endl;
	for(SYMMAP::iterator iter = symMap.begin();iter != symMap.end(); iter++) {
		SYMQUEUE q = iter->second;
		for(int i = 0; i < q->size(); i++) {
			SymBucket *b = (*q)[i];
			cout << "print " << b->getName() << endl;
			b->printBucket(out);
			Symtab * s = b->getNextSymtab();
			if (s != NULL) {
				out << "============begin "<<b->getName() << "'s symtab==================" << endl;
				s->printSymtab(out);
				out << "============end "<<b->getName() << "'s symtab==================" << endl;
			}
		}
	}
	cout << "end printSymtab" << endl;
}
bool cmp(SymBucket *x, SymBucket *y) {
	return y->getOrder() > x->getOrder();
}