# Turing-Welchman Bomb

## Preface
As mentioned before, the `Enigma` wasn't as secure as the german military thought. 
"The Enigma Code" was eventually broken by the allies. I implemented a Turing-Welchman-Bomb.
Here I outline how the Turing-Welchman-Bomb worked. 
If you are already familiar with the way the Bombe worked and you want to see how it's implemented in software, you can click [here].

## Design
### Drums
The Turing-Welchman-Bomb had a special kind of Rotors, that where called Drums. The Drums did have separate wirings: one "normal" and one inverse. 
The reasons for this will later become apperant. The Bomb had three sections consisting out of three rows each containg 12 Drums. 
One column of 3 Rotors simulated an Enigma configuration. 
Because only letter was encrypted/decrypted with each rotor pass, Alan Turing put 12 right next to each other.
Thus enabling him to encrypt/decrypt 12 letters at once. 

