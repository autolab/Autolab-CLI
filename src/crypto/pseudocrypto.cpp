#include "pseudocrypto.h"

#include <cstring>

#include <string>

#include <openssl/err.h>
#include <openssl/evp.h>

#include "logger.h"

#define MAX_CIPHERTEXT_LEN 256

void exit_with_crypto_error() {
  Logger::fatal << "OpenSSL error" << Logger::endl;
  ERR_print_errors_fp(stderr);
  exit(-1);
}

void check_key_and_iv_lengths(unsigned char *key, unsigned char *iv) {
  if (strnlen((char *)key, key_length_in_chars + 1) != key_length_in_chars) {
    Logger::fatal << "[Pseudocrypto] key length error" << Logger::endl;
    exit(-1);
  }
  if (strnlen((char *)iv, iv_length_in_chars + 1) != iv_length_in_chars) {
    Logger::fatal << "[Pseudocrypto] iv length error" << Logger::endl;
    exit(-1);
  }
}

std::string encrypt_string(std::string srctext, unsigned char *key,
    unsigned char *iv) {
  check_key_and_iv_lengths(key, iv);

  EVP_CIPHER_CTX *ctx;
  unsigned char ciphertext[MAX_CIPHERTEXT_LEN];
  int total_len = 0;
  int temp_len = 0;

  unsigned char *plaintext = (unsigned char *)srctext.c_str();
  int input_len = srctext.length();

  // create context
  if (!(ctx = EVP_CIPHER_CTX_new()))
    exit_with_crypto_error();

  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    exit_with_crypto_error();

  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &temp_len, plaintext, input_len))
    exit_with_crypto_error();
  total_len = temp_len;

  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + temp_len, &temp_len))
    exit_with_crypto_error();
  total_len += temp_len;

  EVP_CIPHER_CTX_free(ctx);

  // using std::string with arbitrary bytes is technically allowed
  return std::string((char *)ciphertext, total_len);
}

std::string decrypt_string(char *srctext, size_t srclength, unsigned char *key,
    unsigned char *iv) {
  check_key_and_iv_lengths(key, iv);

  EVP_CIPHER_CTX *ctx;
  unsigned char plaintext[MAX_CIPHERTEXT_LEN];
  int total_len = 0;
  int temp_len = 0;

  unsigned char *ciphertext = (unsigned char *)srctext;
  int input_len = (int)srclength;

  if (!(ctx = EVP_CIPHER_CTX_new()))
    exit_with_crypto_error();

  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    exit_with_crypto_error();

  if (1 != EVP_DecryptUpdate(ctx, plaintext, &temp_len, ciphertext, input_len))
    exit_with_crypto_error();
  total_len = temp_len;

  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + temp_len, &temp_len))
    exit_with_crypto_error();
  total_len += temp_len;

  EVP_CIPHER_CTX_free(ctx);

  return std::string((char *)plaintext, total_len);
}
