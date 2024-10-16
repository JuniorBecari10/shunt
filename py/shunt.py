from dataclasses import dataclass
from typing import Callable
from enum import Enum

class TokenType(Enum):
    Number = 0

    LeftParen = 1
    RightParen = 2

    Plus = 3
    UnaryMinus = 4
    BinaryMinus = 5
    Star = 6
    Slash = 7

@dataclass
class Token:
    lexeme: str
    pos: int
    ty: TokenType

    def __repr__(self) -> str:
        match self.ty:
            case TokenType.UnaryMinus:
                return "u-"
            
            case TokenType.BinaryMinus:
                return "b-"
            
            case _:
                return self.lexeme

class Lexer:
    start: int
    current: int
    tokens: list[Token]

    def __init__(self, expr: str) -> None:
        self.expr = expr
        self.start = 0
        self.current = 0
        self.tokens = []
    
    def next_token(self) -> Token:
        while not self.is_at_end() and self.expr[self.current].isspace():
            self.current += 1

        self.start = self.current

        match self.expr[self.start]:
            case c if c.isdigit():
                while not self.is_at_end() and self.expr[self.current].isdigit():
                    self.current += 1
                
                if not self.is_at_end() and self.expr[self.current] == ".":
                    self.current += 1

                    while not self.is_at_end() and self.expr[self.current].isdigit():
                        self.current += 1

                tk = Token(str(self.expr[self.start:self.current]), self.start, TokenType.Number)
                self.tokens.append(tk)
                return tk
            
            case o:
                self.current += 1

                tk = Token(o, self.start, self.type_of(o))
                self.tokens.append(tk)
                return tk
    
    def type_of(self, lexeme: str) -> TokenType:
        match lexeme:
            case "+": return TokenType.Plus
            case "-":
                if len(self.tokens) == 0 or self.tokens[len(self.tokens) - 1].lexeme in "+-*/": # unary: after start, ( or operator
                    return TokenType.UnaryMinus
                else:
                    return TokenType.BinaryMinus
                    
            case "*": return TokenType.Star
            case "/": return TokenType.Slash

            case "(": return TokenType.LeftParen
            case ")": return TokenType.RightParen

            case _:
                print(f"Error - pos {self.start + 1} - Unknown operator: '{lexeme}'")
                raise Exception()

    def is_at_end(self) -> bool:
        return self.current >= len(self.expr)

def lex_all(expr: str) -> list[Token]:
    l = Lexer(expr)
    tokens: list[Token] = []

    while not l.is_at_end():
        tokens.append(l.next_token())
    
    return tokens

def shunt(expr: str) -> list[Token]:
    l = Lexer(expr)
    output: list[Token] = []
    operators: list[Token] = []

    while not l.is_at_end():
        token = l.next_token()

        match token.ty:
            case TokenType.Number:
                output.append(token)
            
            case TokenType.Plus | TokenType.UnaryMinus | TokenType.BinaryMinus | TokenType.Star | TokenType.Slash:
                while operators[len(operators) - 1].ty.value > token.ty.value if len(operators) > 0 else False:
                    output.append(operators.pop())
                
                operators.append(token)
            
            case TokenType.LeftParen:
                operators.append(token)
            
            case TokenType.RightParen:
                while operators[len(operators) - 1].ty != TokenType.LeftParen if len(operators) > 0 else False:
                    output.append(operators.pop())
                
                if len(operators) > 0:
                    operators.pop()
                else:
                    print("Error - Unmatched parenthesis")
                    raise Exception()
            
    while len(operators) > 0:
        pop = operators.pop()

        if pop.ty == TokenType.LeftParen:
            print("Error - Unmatched parenthesis")
            raise Exception()

        output.append(pop)
    
    return output

def binary(stack: list[float], op: Callable[[float, float], float]) -> None:
    if len(stack) < 2:
        print("Error: Too few items on stack to perform binary operation")
        raise Exception()

    b = stack.pop()
    a = stack.pop()

    stack.append(op(a, b))

def eval(tokens: list[Token]) -> float:
    stack: list[float] = []

    for token in tokens:
        match token.ty:
            case TokenType.Number:
                stack.append(float(token.lexeme))
            
            case TokenType.Plus:
                binary(stack, float.__add__)

            case TokenType.BinaryMinus:
                binary(stack, float.__sub__)
            
            case TokenType.Star:
                binary(stack, float.__mul__)

            case TokenType.Slash:
                binary(stack, float.__truediv__)
            
            case TokenType.UnaryMinus:
                if len(stack) < 1:
                    print("Error: Too few items on stack to perform unary operation")
                    raise Exception()

                stack[len(stack) - 1] = -stack[len(stack) - 1]

            case _:
                print(f"Error - unknown stack item: '{token}'")


    if len(stack) == 1:
        return stack.pop()
    elif len(stack) > 1:
        print("Error - unconnected expressions (help: connect them using an operator)")
        raise Exception()

    # no expression
    raise Exception()


while True:
    inp = input("> ")

    if inp == "exit":
        break

    try:
        sh = shunt(inp)
        res = eval(sh)
        print(int(res) if res % 1 == 0 else res)
    except Exception:
        continue
