# PSU CRYPT
#### Logan Ballard

### Basics
PSUCRYPT Feistel Cipher encryption algorithm based on twofish and skipjack.  It takes a 16 or 20 character key and an arbitrary-length text file and produces an encrypted text file.  Encrypted text files with corresponding keys can be decrypted as well.

### Instructions for Use
- `git clone https://github.com/loganballard/psu-crypt`
- to compile from the command line: `make psucrypt`
- `./psucrypt [1] [2] [3] [4]`
- Argument List:
1. **Encrypt/Decrypt Flag:** '1' for encrypt, '0' for decrypt
2. **Key File:** Relative (or absolute) path to the *key* file.
3. **Read-in File:** Relative (or absolute) path to the file being read in.  For encryption, this will be your plaintext file.  For decryption, this will be your ciphertext file.
4. **Write-out File:** Relative (or absolute) path to the file being written to.  For encryption, this will be your ciphertext file.  For decryption, this will be your (decrypted) plaintext file.

# Examples:

### Encryption:

- key.txt: 
> 69c0689476a8fbc9a56b
- plain.txt: 
> 50. The conservatives are fools: They whine about the decay of traditional values, yet they enthusiastically support technological progress and economic growth. Apparently it never occurs to them that you cant make rapid, drastic changes in the technology and the economy of a society without causing rapid changes in all other aspects of the society as well, and that such rapid changes inevitably break down traditional values.
- `./psucrypt 1 test/key.txt test/plain.txt test/cipher.txt`
- Output: 
> 47d7dd42a85f80d359bb0d741f2961eca04703f2cce832aed46204e7c73ae00ec1f351c54d988568107336f5a10420daac79ef531baff9f1dcec31336f26d746997e42c85ab4c11217175438c33d0e4c13467e28826b8bb35832f60dfa3117b2cab9e708e813e7a1c2aa4b2122af95cef00122fcaa8ed092120b39c8527cfcd1ffbca6dfeb6312b5aefe187c6b0f152e06cb7d8a02c7b4085178d1f851d530ef2eddc5eeae21ea6499f80f00516fcb70f47254d3b61b609ac6625b0e3c5c6115a102c18f5d5cedc272e838c5ab0a83d5e697754e6117d21b9b0cf30ef56249d079b7c1f03cd5220545d5a695a3c969f2cbbc4762dbc9ed2a8e310e747800cca1bef70777953b611eecb5c31aac4ccfdf45f859e07762d55b329297856321e6634032a6b65ba4c5bb104c16cc3aba98225c4cbff0fc1785c8fdbe1740ada4800f43280e60ac37a745fc5a9c1dfbfb5304173694daba1f8be0f258f99ce70e6101be6e377fcd8346a6c25111d7a1f909093ee0e02a83415e3f12ec96678ac9461614c597a2f35da6f3fad23ebd04d48fcef647ec47ae9e8f16cefbbab71beeb7cc4a7be7c60aacd4b2d11c0b69d2ab9ff000b790d7a96ef364


### Decryption:
- key.txt: 
> 69c0689476a8fbc9a56b
- cipher.txt:
> 47d7dd42a85f80d359bb0d741f2961eca04703f2cce832aed46204e7c73ae00ec1f351c54d988568107336f5a10420daac79ef531baff9f1dcec31336f26d746997e42c85ab4c11217175438c33d0e4c13467e28826b8bb35832f60dfa3117b2cab9e708e813e7a1c2aa4b2122af95cef00122fcaa8ed092120b39c8527cfcd1ffbca6dfeb6312b5aefe187c6b0f152e06cb7d8a02c7b4085178d1f851d530ef2eddc5eeae21ea6499f80f00516fcb70f47254d3b61b609ac6625b0e3c5c6115a102c18f5d5cedc272e838c5ab0a83d5e697754e6117d21b9b0cf30ef56249d079b7c1f03cd5220545d5a695a3c969f2cbbc4762dbc9ed2a8e310e747800cca1bef70777953b611eecb5c31aac4ccfdf45f859e07762d55b329297856321e6634032a6b65ba4c5bb104c16cc3aba98225c4cbff0fc1785c8fdbe1740ada4800f43280e60ac37a745fc5a9c1dfbfb5304173694daba1f8be0f258f99ce70e6101be6e377fcd8346a6c25111d7a1f909093ee0e02a83415e3f12ec96678ac9461614c597a2f35da6f3fad23ebd04d48fcef647ec47ae9e8f16cefbbab71beeb7cc4a7be7c60aacd4b2d11c0b69d2ab9ff000b790d7a96ef364
- `./psucrypt 0 test/key.txt test/cipher.txt test/plain.txt`
- plain.txt: 
> 50. The conservatives are fools: They whine about the decay of traditional values, yet they enthusiastically support technological progress and economic growth. Apparently it never occurs to them that you cant make rapid, drastic changes in the technology and the economy of a society without causing rapid changes in all other aspects of the society as well, and that such rapid changes inevitably break down traditional values.