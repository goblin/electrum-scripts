#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <openssl/evp.h>

const int NUM_ITER = 2048;
#define DIGEST_ALGO EVP_sha512

void print_in_hex(const unsigned char *buf, size_t buflen)
{
	size_t i;
	for(i = 0; i < buflen; i++) {
		printf("%02x", buf[i]);
	}
	printf("\n");
}

int print_pbkdf2(const char *pwd, size_t pwdlen, const unsigned char *salt, int saltlen)
{
	const int OUTLEN = 64;
	unsigned char out[OUTLEN];
	int rv;

	memset(out, 0, OUTLEN);

	// just in case it's a secret too...
	if(mlock(out, OUTLEN) != 0) {
		perror("unable to mlock()");
		return 4;
	}

	rv = PKCS5_PBKDF2_HMAC(pwd, pwdlen,
			salt, saltlen, NUM_ITER,
			DIGEST_ALGO(),
			OUTLEN, out);

	if(rv != 1) {
		fprintf(stderr, "openssl returned an error\n");
		memset(out, 0, OUTLEN);
		return 5;
	}

	print_in_hex(out, OUTLEN);
	memset(out, 0, OUTLEN);

	return 0;
}

int main(int argc, char **argv)
{
	const int BUFLEN = 256;
	char buf[BUFLEN];
	size_t num_read;
	int rv;

	if(argc < 2) {
		fprintf(stderr, "need salt as an argument\n");
		return 1;
	}

	if(mlock(buf, BUFLEN) != 0) {
		perror("unable to mlock()");
		return 2;
	}

	memset(buf, 0, BUFLEN);

	num_read = fread(buf, 1, 256, stdin);
	if(num_read == 0 || num_read > 254) {
		fprintf(stderr, "invalid password length\n");
		return 3;
	}

	rv = print_pbkdf2(buf, num_read, (unsigned char*)argv[1], strlen(argv[1]));
	memset(buf, 0, BUFLEN);

	return rv;
}
