# Enigma

## Preface

The Enigma-Machine is a cipher device developed in the early 20th century.
It was primarily used in the second world war by the german army to encrypt/decrypt messages.
Most Enigmas where equipped with three rotors and a plugboard,
although variants with four rotors where built at the end of the second world war.
This implementation supports Enigmas with three and four rotors.

## Design

### Rotor 

Each rotor of the Enigma performed a mono alphabetic substitution, which is nothing else than a letter substitution.

#### Wiring

Here is the wiring of the Rotor I:

| A | B | C | D | E | F | G | H | I | J | K | L | M | N | O | P | Q | R | S | T | U | V | W | X | Y | Z |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| E | K | M | F | L | G | D | Q | V | Z | N | T | O | W | Y | H | X | U | S | P | A | I | B | R | C | J |

When current is applied to the letter 'A' at the rotor input, the internal wiring substitutes it to the letter 'E'.
Because a substitution cipher is very unsafe, we permute the rotor postion by rotating it with each keypress.
So next time we press an 'A'
the input contact is still at the same place, but now the rotor has advanced forward one place. 
The current of the letter 'A' now takes the path of the rotor letter 'B'. 
The rotation takes place before each keypress.
So if the starting position of the rotor was to be 'A',
and we were to encrypt the letter 'A', it would actually take the path of 'B'.

Because this is still not secure enough, an Enigma was typically equipped with three rotors. 
They rotated according to the scheme of a mechanical odometer or like the pointers of a clock.
So if a rotor has made a complete advance, the rotor left to it advances forward one place.

#### Ring Setting

The turning point could also be set through the "Ring Setting".
It changed the relation of the internal wiring to the outer visible letters.
It also rotated a notch, which was responsible for turning the next rotor over. 

There was an anomaly in the turning procedure:
If the left most rotor (the slowest) advanced one step forward, it always took the middle one with it. 
Meaning all rotors rotated at the same time.

#### Reflector

As a final complication, each Enigma did have a "Reflector".
A special kind of Rotor
that never rotated but also performed a letter substitution and redirected the current back in rotors again.

#### Current path

So the current passed through the rotors twice:
the first time from right to left in the reflector and then from left to right.

A three-rotor Enigma had a total of five possible rotors available,
and the four rotor variants had a total of 8 available. 

## Secure enough?

The first commercial Enigmas where equipped without a plugboard, which is discussed latter.
The german army asked itself the question: How secure is the Enigma?

So let's calculate how secure an Enigma without a plugboard is:

The number of possible rotor arrangements for a three-rotor Enigma is:

$$
P(5, 3) = \frac{5!}{(5-3)!} = 60 \text{ Rotor Permutations}
$$

Combined with the 26 possible positions of each rotor:

$$
26^3=17.576 \cdot 60 = 1.054.560 \text{ Theoretical Combinations}
$$

Although $1.054.560$ combinations seem like a large number,
it is a very, very small number in cryptology terms and with the help of machines even in 1939 "bruteforce-able".


### Plugboard

The german army realized this problem too, so they added the "Plugboard".
It consisted of 26 "plug holes" one for each letter.
A typical Enigma was equipped with 10 plugboard cables.
This plugboard also performs a letter substitution and has commutative properties meaning:
Plugging the 'A' and 'B' is the same as plugging 'B' and 'A'.

So let's calculate the combinations:

$$
\frac{26!}{6!\cdot 10!\cdot 2^{10}} = 150.738.274.937.250
$$

$26!$ are the possible permutations of the alphabet meaning in how many ways we can arrange 26 letters.

Because there were typically only 10 plugboard cables provided, and each of them formed a pair, there were six letters leftover.
Meaning we must divide by $6!$.

Because the order of these ten pairs doesn't matter, we divide by $10!$.

And lastly, we must pay attention to the commutative properties of the plugboard.
Because we have ten cables, the number of combinations we must rule out is $2^{10}$.

To calculate the actual number of combinations, we must consider the anomaly described in "Ring Setting":

To rule out these anomalies, we must first calculate the number of ring settings:

$$
26^2=676 \text{ Ring Positions}
$$

Why $26^2$?
Because the notch of the ring setting of the left most rotor doesn't rotate any other rotors,
we must only calculate the combinations for two of these rotors.

There were only 26 positions in which the above-described anomaly didn't occur,
so we must eliminate the following combinations:

$$
26^2−26 = 650 \text{ Redundant Positions}
$$

So the actual number of possible combinations is:

$$
26^3–650 = 16.926 \text{ Actual Rotor Positions}
$$

The resulting number is hard to comprehend:

$$
60 \cdot 676 \cdot 16.926 \cdot 150.738.274.937.250 = 103.484.623.446.804.960.360.000 \text{ Actual Combinations}
$$

In words:
one hundred three sextillion four hundred eighty-four quintillion six hundred twenty-three quadrillion four hundred 
forty-six trillion eight hundred four billion nine hundred sixty million three hundred sixty thousand combinations.

This is roughly $10^{23}$ combinations.
It should be apparent that this is not "bruteforce-able".
The key length is about 76 bits,
which for the time was enormous and still is quite impressive even for today's standards.
The wildly used encryption algorithm [DES](https://en.wikipedia.org/wiki/Data_Encryption_Standard) has comparatively small 
key length of only 56 bits.
Although key length is not a maxime for security,
because a mono alphabetic substitution has a key length of 88 bits and is much unsafer, it gives a rough direction.

## This has to be Secure enough! Right?

All that glitters is not gold.
This enormous key length isn't what the allies needed to break.
Because the plugboard just performs a mono alphabetic substitution
and doesn't change during the whole encryption process,
it can be ruled out with intelligent cryptanalytic attacks like the [Turing Bomb](TuringBomb.md).
So we can cancel out the factor $150.738.274.937.250$. 
The ring settings were also quite useless
because messages had an upper limit of 250 characters, and the middle rotor
which had a periodic length of 650, the anomaly only happened in rare cases, thus it could be neglected. 
Overall, the ring settings caused no trouble for the cryptanalyst of the second world war.  
So we can cancel the factor $676$. 

$$
60 \cdot 17.576 = 1.054.560
$$

After all this, we are left with compared to before a laughable key length of $1.054.560$.


**Side-note**:
The sometimes mentioned 150 million, million, million possible Enigma settings leave out the ring settings.

