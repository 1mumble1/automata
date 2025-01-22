import sys
import re

class Token:
    def __init__(self, type_, lexeme, line, column):
        self.type = type_
        self.lexeme = lexeme
        self.line = line
        self.column = column

    def __str__(self):
        return f"{self.type} ({self.line}, {self.column}) \"{self.lexeme}\""

class PascalLexer:
    def __init__(self, filename):
        self.file = open(filename, 'r')
        self.line = 0
        self.column = 0
        self.current_line = ""
        self.keywords = {"array", "begin", "else", "end", "if", "of", "or",
                         "program", "procedure", "then", "type", "var"}
        self.operators = {"*", "+", "-", "/", ";", ",", "(", ")", "[", "]", "=", ">", "<", "<=", ">=", "<>", ":", ":=", "."}
        self.separators = {"\"", "(", ")", "+", "-", "\t", "\n", " ", ";", ",", ".", "[", "]", "{", "}", "*", "/", "'", ":"}

    def next_token(self):
        while True:
            if not self.current_line:
                self.current_line = self.file.readline()
                if not self.current_line:
                    return None
                self.line += 1
                self.column = 0

            while self.column < len(self.current_line):
                char = self.current_line[self.column]

                # пропуск пробельных символов
                if char.isspace():
                    self.column += 1
                    continue

                # пропуск линейных комментов
                if char == "/" and self.peek(1) == "/":
                    self.read_until("\n")
                    continue

                # пропуск блочных комментов до }, иначе отправляем токен BAD
                if char == "{":
                    is_valid_block_comment = False
                    block_comment = char
                    start_line = self.line
                    start_col = self.column
                    self.column += 1
                    while not is_valid_block_comment:
                        if self.column >= len(self.current_line):
                            self.current_line = self.file.readline()
                            if not self.current_line:
                                return Token("BAD", block_comment, start_line, start_col)
                            self.line += 1
                            self.column = 0
                            continue

                        char = self.current_line[self.column]
                        block_comment += char
                        self.column += 1

                        if char == "}":
                            is_valid_block_comment = True
                
                # строковые литералы
                if char == "'":
                    start_col = self.column
                    lexeme = self.read_string()
                    if lexeme[len(lexeme) - 1] != "'":
                        return Token("BAD", lexeme.strip("\n"), self.line, start_col + 1)
                    return Token("STRING", lexeme, self.line, start_col + 1)

                # целые числа либо числа с плавающей точкой
                if char.isdigit():
                    start_col = self.column
                    lexeme = self.read_while(lambda c: c not in self.separators)
                    if not lexeme.isdigit():
                        return Token("BAD", lexeme, self.line, start_col + 1)
                    if self.peek(0) == '.':
                        self.column += 1
                        float_part = self.read_while(lambda c: c not in self.separators)
                        if not float_part:
                            return Token("BAD", lexeme + '.', self.line, start_col + 1)
                        if self.peek(0) in {'+', '-'}:
                            sign = self.peek(0)
                            self.column += 1
                            range_of_exp = self.read_while(lambda c: c not in self.separators)
                            if not range_of_exp:
                                return Token("BAD", lexeme + '.' + float_part + sign, self.line, start_col + 1)
                            float_part += sign + range_of_exp
                        match = re.fullmatch(r"\d+(\.\d+)?([eE][+-]?\d+)?", lexeme + '.' + float_part)
                        if match:
                            return Token("FLOAT", lexeme + '.' + float_part, self.line, start_col + 1)
                        else:
                            return Token("BAD", lexeme + '.' + float_part, self.line, start_col + 1)

                    if len(lexeme) > 16:
                        return Token("BAD", lexeme, self.line, start_col + 1)
                    return Token("INTEGER", lexeme, self.line, start_col + 1)

                # идентификатор либо ключевое слово
                if char.isalpha() or char == "_":
                    start_col = self.column
                    lexeme = self.read_while(lambda c: c not in self.separators)
                    match = re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", lexeme)
                    if not match:
                        return Token("BAD", lexeme, self.line, start_col + 1)
                    token_type = lexeme.upper() if lexeme.lower() in self.keywords else "IDENTIFIER"
                    return Token(token_type, lexeme, self.line, start_col + 1)

                # оператор
                if char in self.operators:
                    start_col = self.column
                    lexeme = self.read_operator()
                    token_type = self.get_operator_type(lexeme)
                    return Token(token_type, lexeme, self.line, start_col + 1)

                # ошибочная лексема
                start_col = self.column
                bad_lexeme = self.read_while(lambda c: c not in self.separators)
                return Token("BAD", bad_lexeme, self.line, start_col + 1)

            self.current_line = ""
                  

        #         # Ключевые слова и идентификаторы
        #         if char.isalpha() or char == "_":
        #             start_col = self.column
        #             lexeme = self.read_while(lambda c: c.isalnum() or c == "_")
        #             token_type = lexeme.upper() if lexeme.lower() in self.keywords else "IDENTIFIER"
        #             return Token(token_type, lexeme, self.line, start_col + 1)

        #         # Числа (целые и вещественные)
        #         if char.isdigit():
        #             start_col = self.column
        #             lexeme = self.read_while(lambda c: c.isdigit() or c == ".")
        #             if self.peek(0) == 'e' and self.peek(1) in {'+', '-'}:
        #                 lexeme += 'e' + self.peek(1)
        #                 self.column += 2
        #                 float_range = self.read_while(lambda c: c.isdigit())
        #                 if not float_range:
        #                     return Token("BAD", lexeme, self.line, start_col + 1)
        #                 return Token("FLOAT", lexeme + float_range, self.line, start_col + 1)

        #             if ".." in lexeme:
        #                 self.column -= len(lexeme) - lexeme.index("..")
        #                 lexeme = lexeme[:lexeme.index("..")]
        #             if lexeme.count(".") > 1:
        #                 return Token("BAD", lexeme, self.line, start_col + 1)
        #             if "." not in lexeme and int(lexeme) > 32_767:
        #                 return Token("BAD", lexeme, self.line, start_col + 1)
        #             token_type = "FLOAT" if "." in lexeme else "INTEGER"
        #             return Token(token_type, lexeme, self.line, start_col + 1)

        #         # Операторы и знаки пунктуации
        #         if char in self.operators:
        #             start_col = self.column
        #             lexeme = self.read_operator()
        #             token_type = self.get_operator_type(lexeme)
        #             return Token(token_type, lexeme, self.line, start_col + 1)

        #         # Некорректные токены
        #         start_col = self.column
        #         self.column += 1
        #         return Token("BAD", char, self.line, start_col + 1)

        #     self.current_line = ""  # Переход к следующей строке

    def read_while(self, condition):
        start = self.column
        while self.column < len(self.current_line) and condition(self.current_line[self.column]):
            self.column += 1
        return self.current_line[start:self.column]

    def read_operator(self):
        start = self.column
        self.column += 1
        if self.column < len(self.current_line) and self.current_line[start:self.column + 1] in self.operators:
            self.column += 1
        return self.current_line[start:self.column]

    def get_operator_type(self, lexeme):
        return {
            "+": "PLUS", "-": "MINUS", "*": "MULTIPLICATION", "/": "DIVIDE",
            ";": "SEMICOLON", ",": "COMMA", "(": "LEFT_PAREN", ")": "RIGHT_PAREN",
            "[": "LEFT_BRACKET", "]": "RIGHT_BRACKET", "=": "EQ", ">": "GREATER",
            "<": "LESS", "<=": "LESS_EQ", ">=": "GREATER_EQ", "<>": "NOT_EQ",":": "COLON", ":=": "ASSIGN", ".": "DOT"
        }.get(lexeme, "BAD")

    def read_string(self):
        start = self.column
        self.column += 1  # Пропустить начальную кавычку
        while self.column < len(self.current_line) and self.current_line[self.column] != "'":
            self.column += 1
        self.column += 1  # Пропустить конечную кавычку
        return self.current_line[start:self.column]

    def read_until(self, end_char):
        start = self.column
        self.column += 1  # Пропустить начальный символ комментария
        while self.column < len(self.current_line) and self.current_line[self.column] != end_char:
            self.column += 1
        if self.column < len(self.current_line):  # Пропустить конечный символ комментария
            self.column += 1
        return self.current_line[start:self.column]

    def peek(self, offset):
        if self.column + offset < len(self.current_line):
            return self.current_line[self.column + offset]
        return ""

    def close(self):
        self.file.close()

def main():
    if len(sys.argv) != 3:
        print(f"Usage: python {sys.argv[0]} <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    lexer = PascalLexer(input_file)

    with open(output_file, 'w') as output:
        while True:
            token = lexer.next_token()
            if token is None:
                break
            print(token)
            output.write(str(token) + '\n')

    lexer.close()

if __name__ == "__main__":
    main()
