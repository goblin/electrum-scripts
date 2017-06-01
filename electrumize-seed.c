#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <openssl/hmac.h>

const int MAX_ITER = 9999;
const int MAX_EXTENSION_LEN = 5;
const int MAX_SEEDLEN = 1024;
const char *HMAC_KEY = "Seed version";

int test(const char *buf, int buflen)
{
	return HMAC(EVP_sha512(), HMAC_KEY, strlen(HMAC_KEY), (unsigned char*)buf, buflen, NULL, NULL)[0] == 1;
}

int main()
{
	const int BUFLEN = MAX_SEEDLEN + MAX_EXTENSION_LEN + 1;
	char buf[BUFLEN];
	size_t num_read;
	int i;

	if(mlock(buf, BUFLEN) != 0) {
		perror("unable to mlock()");
		return 1;
	}

	memset(buf, 0, BUFLEN);

	num_read = fread(buf, 1, MAX_SEEDLEN, stdin);
	if(num_read == 0 || num_read > MAX_SEEDLEN - 2) {
		fprintf(stderr, "invalid seed length\n");
		return 2;
	}

	if(test(buf, num_read)) {
		fwrite(buf, 1, num_read, stdout);
		printf("\n");
		memset(buf, 0, BUFLEN);
		return 0;
	}

	for(i = 0; i < MAX_ITER; i++) {
		int added = snprintf(buf + num_read, MAX_EXTENSION_LEN, " %d", i);
		if(test(buf, num_read + added)) {
			printf("%s\n", buf);
			memset(buf, 0, BUFLEN);
			return 0;
		}
	}
	
	memset(buf, 0, BUFLEN);

	fprintf(stderr, "no extension found... very unlucky?\n");
	return 3;
}
