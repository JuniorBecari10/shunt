#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "shunt.h"

int main() {
    repl();
    return 0;
}

void repl() {
  for (;;) {
    char buffer[MAX_LEN];
    double out;

    if (
      !input("> ", buffer) ||
      strcmp(buffer, "exit") == 0
    )
      break;

    if (!calculate(buffer, &out))
      continue;

    print_double(out);
  }
}

bool calculate(char *buffer, double *out) {
  Token tokens[MAX_LEN];
  int tokens_len = 0;

  if (
    !shunt(buffer, tokens, &tokens_len) ||
    !eval(tokens, tokens_len, out)
  )
    return false;

  return true;
}

bool input(const char* prompt, char *buffer) {
  printf("%s", prompt);
  return read_line(buffer);
}

bool read_line(char *buffer) {
  if (fgets(buffer, MAX_LEN, stdin) != NULL) {
    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n')
      buffer[len - 1] = '\0';

    return true;
  }

  else {
    printf("Error - an error occurred while reading input or EOF reached.\n");
    return false;
  }
}

Token new_token(char *lexeme, int len, int pos, TokenType type) {
    return (Token) {
        .lexeme = lexeme,
        .lexeme_len = len,

        .pos = pos,
        .type = type,
    };
}

void print(Token *t) {
    printf("%d - ", t->type);
    switch (t->type) {
        case TypeUnaryMinus: {
            printf("u-\n");
            break;
        }

        case TypeBinaryMinus: {
            printf("b-\n");
            break;
        }
        
        default: {
            printf("%.*s\n", t->lexeme_len, t->lexeme); 
            break;
        }
    }
}

double to_double(Token *t) {
  char buffer[MAX_LEN];

  if (t->lexeme_len >= sizeof(buffer)) {
    printf("Error - lexeme too long to convert to double\n");
    return 0.0;
  }

  strncpy(buffer, t->lexeme, t->lexeme_len);

  buffer[t->lexeme_len] = '\0';

  char *end;
  double value = strtod(buffer, &end);

  if (end == buffer) {
    printf("Error - invalid number in lexeme\n");
    return 0.0;
  }

  return value;
}

Lexer new_lexer(char *expr) {
    return (Lexer) {
        .expr = expr,
        .expr_len = strlen(expr),

        .start = 0,
        .current = 0,

        .last = (Token) { .pos = -1 }, // sentinel to describe absence
    };
}

Token next_token(Lexer *l) {
    #define IS_DIGIT(c) c >= '0' && c <= '9'

    while (!is_at_end(l) && l->expr[l->current] == ' ')
        l->current++;
    
    l->start = l->current;
    char s = l->expr[l->start];

    if (IS_DIGIT(s)) {
        while (!is_at_end(l) && IS_DIGIT(l->expr[l->current]))
            l->current++;
        
        if (!is_at_end(l) && l->expr[l->current] == '.') {
            l->current += 1;

            while (!is_at_end(l) && IS_DIGIT(l->expr[l->current]))
                l->current++;
        }

        Token tk = new_token(&l->expr[l->start], l->current - l->start, l->start, TypeNumber);
        l->last = tk;

        return tk;
    }
    else {
        l->current += 1;

        Token tk = new_token(&l->expr[l->start], 1, l->start, type_of(l, l->expr[l->start]));
        l->last = tk;

        if (tk.type == TypeNone)
          return (Token) { .pos = -1 };
        
        return tk;
    }

    #undef IS_DIGIT
}

TokenType type_of(Lexer *l, char lexeme) {
  switch (lexeme) {
    case '+': return TypePlus;
    case '-':
      #define LAST_LEXEME_CHAR l->last.lexeme[0]

      if (l->last.pos == -1
        || LAST_LEXEME_CHAR == '+'
        || LAST_LEXEME_CHAR == '-'
        || LAST_LEXEME_CHAR == '*'
        || LAST_LEXEME_CHAR == '/')
        return TypeUnaryMinus;

      else
        return TypeBinaryMinus;

      #undef LAST_LEXEME


    case '*': return TypeStar;
    case '/': return TypeSlash;

    case '(': return TypeLeftParen;
    case ')': return TypeRightParen;

    default:
      printf("Error - pos %d - Unknown operator: '%c'\n", l->start + 1, lexeme);
      return TypeNone;
  }
}

bool is_at_end(Lexer *l) {
    return l->current >= l->expr_len;
}

bool shunt(char *expr, Token *return_arr, int *return_arr_len) {
    #define PUSH(list, element) list[(*list##_len)++] = element
    #define POP(list) list[--(*list##_len)]

    Lexer l = new_lexer(expr);

    Token operators[MAX_LEN];
    int ol = 0;
    int *operators_len = &ol;

    while (!is_at_end(&l)) {
        Token t = next_token(&l);

        if (t.pos == -1)
            return false;

        switch (t.type) {
            case TypeNumber: {
                PUSH(return_arr, t);
                break;
            }

            case TypePlus:
            case TypeUnaryMinus:
            case TypeBinaryMinus:
            case TypeStar:
            case TypeSlash: {
                while (*operators_len > 0 && operators[*operators_len - 1].type >= t.type) {
                    Token op = POP(operators);
                    PUSH(return_arr, op);
                }

                PUSH(operators, t);
                break;
            }

            case TypeLeftParen: {
                PUSH(operators, t);
                break;
            }

            case TypeRightParen: {
                while (*operators_len > 0 && operators[*operators_len - 1].type != TypeLeftParen) {
                    Token op = POP(operators);
                    PUSH(return_arr, op);
                }

                if (*operators_len > 0)
                    (void) POP(operators);
                else {
                    printf("Error - Unmatched parenthesis\n");
                    return false;
                }
                break;
            }

            case TypeNone: {
                return false;
            }
        }
    }

    while (*operators_len > 0) {
        Token pop = POP(operators);

        if (pop.type == TypeLeftParen) {
            printf("Error - Unmatched parenthesis\n");
            return false;
        }

        PUSH(return_arr, pop);
    }

    #undef PUSH
    #undef POP

    return true;
}

bool eval(Token *tokens, int len, double *out) {
  #define PUSH(e) stack[sp++] = e
  #define POP() stack[--sp]

  #define BINARY(op)                                                            \
    do {                                                                        \
      if (sp < 2) {                                                             \
        printf("Error - too few items on stack to perform binary operation\n"); \
        return false;                                                           \
      }                                                                         \
                                                                                \
      double b = POP();                                                         \
      double a = POP();                                                         \
                                                                                \
      PUSH(a op b);                                                             \
    } while (false)

  double stack[MAX_LEN];
  int sp = 0;

  for (int i = 0; i < len; i++) {
    Token t = tokens[i];

    switch (t.type) {
      case TypeNumber: {
        PUSH(to_double(&t));
        break;
      }

      case TypePlus: {
        BINARY(+);
        break;
      }

      case TypeBinaryMinus: {
        BINARY(-);
        break;
      }

      case TypeStar: {
        BINARY(*);
        break;
      }

      case TypeSlash: {
        BINARY(/);
        break;
      }

      case TypeUnaryMinus: {
        if (sp < 1) {
          printf("Error - too few items on stack to perform unary operation.\n");
          return false;
        }

        stack[sp - 1] = -stack[sp - 1];
        break;
      }

      default: {
        printf("Error - unknown stack item: ");
        print(&t);
        
        return false;
      }
    }
  }

  if (sp == 1) {
    *out = POP();
    return true;
  }

  else if (sp > 1) {
    printf("Error - unconnected expressions.\n");
    return false;
  }

  #undef PUSH
  #undef POP
  #undef BINARY

  // no expression
  return false;
}

void print_double(double v) {
  if (v == (int) v)
    printf("%d\n", (int) v);
  else
    printf("%.2f\n", v);
}

