#ifndef TOKENS_H
#define TOKENS_H

void tokenize(const char* input, const char* delimiter, char*** tokens, int* token_count);
void free_tokens(char** tokens, int token_count);

#endif
