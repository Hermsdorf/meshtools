#ifndef HASH_H__
#define HASH_H__

#include <iostream>
#include <vector>

// Hash function to fill face_to_element array
// Referencia: cantor pairing function
// http://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
unsigned long long cantor_pairing(unsigned long long a, unsigned long long b);

unsigned long  compute_hash(std::vector<unsigned int> conn);

#endif /* HASH_H__ */
