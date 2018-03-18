/*
 * Simple helpers for quickly encrypting and decrypting strings using
 * AES 256 CBC.
 *
 * 'Pseudo' is used in the name because it's not really completely secure, but
 * good enough to obfuscate data for non-malicious users.
 */

#ifndef AUTOLAB_PSEUDOCRYPTO_H_
#define AUTOLAB_PSEUDOCRYPTO_H_

#include <cstddef>

#include <string>

const std::size_t key_length_in_chars = 32;
const std::size_t iv_length_in_chars = 16;

std::string encrypt_string(std::string srctext, unsigned char *key,
    unsigned char *iv);
std::string decrypt_string(char *srctext, size_t srclength, unsigned char *key,
    unsigned char *iv);

#endif /* AUTOLAB_PSEUDOCRYPTO_H_ */