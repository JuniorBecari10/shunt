#define MAX_LEN 255

typedef enum {
    TypeNumber,

    TypeLeftParen,
    TypeRightParen,

    TypePlus,
    TypeUnaryMinus,
    TypeBinaryMinus,
    TypeStar,
    TypeSlash,

    TypeNone,
} TokenType;

typedef struct {
    char *lexeme;
    int lexeme_len;

    int pos;
    TokenType type;
} Token;

typedef struct {
    char *expr;
    int expr_len;

    int start;
    int current;

    Token last;
} Lexer;

Token new_token(char *lexeme, int len, int pos, TokenType type);
void print(Token *t);
double to_double(Token *t);

Lexer new_lexer(char *expr);
Token next_token(Lexer *l);
bool is_at_end(Lexer *l); 
TokenType type_of(Lexer *l, char lexeme);

bool shunt(char *expr, Token *return_arr, int *return_len);
bool eval(Token *tokens, int len, double *out);

void print_double(double v);

void repl();
bool calculate(char *buffer, double *out);
bool input(const char* prompt, char *buffer);
bool read_line(char *buffer);

