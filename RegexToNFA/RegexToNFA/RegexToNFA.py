import sys

class Tree:
    def __init__(self, value, left=None, right=None):
        self.value = value
        self.left = left
        self.right = right

    def __repr__(self, level=0):
        ret = "\t" * level + f"{self.value}\n"
        if self.left:
            ret += f"Left of {self.value}:\n"
            ret += self.left.__repr__(level + 1)
        if self.right:
            ret += f"Right of {self.value}:\n"
            ret += self.right.__repr__(level + 1)
        return ret

class Graph:
    def __init__(self, state, children=None):
        self.state = state
        self.children = children if children is not None else []

class Parser:
    def __init__(self, expression):
        self.expression = expression
        self.index = 0
        self.len = len(expression)
        self.current_char = self.expression[self.index] if self.len > 0 else ''
        self.token = None

    def error(self, message):
        raise Exception(message)

    def advance(self):
        self.index += 1
        if self.index < self.len:
            self.current_char = self.expression[self.index]
        else:
            self.current_char = ''

    def get_next_token(self):
        while self.current_char:
            if self.current_char.isspace():
                token = ' '
                self.advance()
                return token
            if self.current_char in {'|', '*', '+', '(', ')'}:
                token = self.current_char
                self.advance()
                return token
            if self.current_char.isalnum():
                token = self.current_char
                self.advance()
                return token
            self.error(f"Unexpected character: {self.current_char}")
        return None

    def parse(self):
        self.token = self.get_next_token()
        tree = self.parse_expression()
        if self.token is not None:
            self.error("Unexpected token at the end.")
        return tree

    def parse_expression(self):
        tree = self.parse_term()
        while self.token == '|':
            self.token = self.get_next_token()
            right = self.parse_term()
            tree = Tree('|', tree, right)
        return tree

    def parse_term(self):
        tree = self.parse_factor()
        while self.token and self.token not in {'|', ')'}:
            right = self.parse_factor()
            tree = Tree('.', tree, right)
        return tree

    def parse_factor(self):
        base = self.parse_base()
        if self.token == '*':
            self.token = self.get_next_token()
            return Tree('*', base)
        elif self.token == '+':
            self.token = self.get_next_token()
            return Tree('+', base)
        else:
            return base

    def parse_base(self):
        if self.token == '(':
            self.token = self.get_next_token()
            if self.token == ')':
                # Empty symbol ()
                self.token = self.get_next_token()
                return Tree('epsilon')
            tree = self.parse_expression()
            if self.token != ')':
                self.error("Expected ')'")
            self.token = self.get_next_token()
            return tree
        elif self.token is not None and self.token not in {'|', '*', '+', '(', ')'}:
            token_value = self.token
            self.token = self.get_next_token()
            return Tree(token_value)
        else:
            self.error(f"Unexpected token: {self.token}")

def add_child(entry, father, child, alphabet):
    if entry not in alphabet:
        alphabet.append(entry)
    for c in father.children:
        if c["entry"] == entry:
            c["states"].append(child)
            break
    else:
        father.children.append({"entry": entry, "states": [child]})

def new_state(counter):
    counter += 1
    return Graph(f"q{counter}"), counter

def build_nfa_from_tree(parse_tree_root, start, finite, alphabet, counter):
    if parse_tree_root.value.isalnum():
        add_child(parse_tree_root.value, start, finite, alphabet)

        return

    elif parse_tree_root.value == '.':
        new_start, counter = new_state(counter)
        new_finite, counter = new_state(counter)

        build_nfa_from_tree(parse_tree_root.left, start, new_start, alphabet, counter)
        build_nfa_from_tree(parse_tree_root.right, new_finite, finite, alphabet, counter)

        add_child('ε', new_start, new_finite, alphabet)

        return

    elif parse_tree_root.value == '|':
        new_start_left, counter = new_state(counter)
        new_finite_left, counter = new_state(counter)
        new_start_right, counter = new_state(counter)
        new_finite_right, counter = new_state(counter)

        build_nfa_from_tree(parse_tree_root.left, new_start_left, new_finite_left, alphabet, counter)
        build_nfa_from_tree(parse_tree_root.right, new_start_right, new_finite_right, alphabet, counter)

        add_child('ε', start, new_start_left, alphabet)
        add_child('ε', start, new_start_right, alphabet)
        add_child('ε', new_finite_left, finite, alphabet)
        add_child('ε', new_finite_right, finite, alphabet)

        return

    elif parse_tree_root.value == '*':
        new_start, counter = new_state(counter)
        new_finite, counter = new_state(counter)

        build_nfa_from_tree(parse_tree_root.left, new_start, new_finite, alphabet, counter)

        add_child('ε', start, new_start, alphabet)
        add_child('ε', new_finite, finite, alphabet)
        add_child('ε', new_finite, new_start, alphabet)
        add_child('ε', start, finite, alphabet)

        return

    elif parse_tree_root.value == '+':
        new_start, counter = new_state(counter)
        new_finite, counter = new_state(counter)

        build_nfa_from_tree(parse_tree_root.left, new_start, new_finite, alphabet, counter)

        add_child('ε', start, new_start, alphabet)
        add_child('ε', new_finite, finite, alphabet)
        add_child('ε', new_finite, new_start, alphabet)

        return

    return

def get_transits(start):
    states = []
    transitions = {}

    q = []
    q.append(start)
    while q:
        state = q.pop(0)
        states.append(state.state)
        transitions[state.state] = []

        for child in state.children:
            transitions[state.state].append({"entry": child["entry"], "states": []})
            for c in child["states"]:
                transitions[state.state][len(transitions[state.state]) - 1]["states"].append(c.state)
                if c.state not in states and c not in q:
                    q.append(c)

    #print(transitions)
    return states, transitions

def build_table_nfa(start, finite, alphabet):
    nfa = {
        "entries": alphabet,
        "states_with_transitions": []
        }

    states, transitions = get_transits(start)
    for s in states:
        state = {
            "current_state": s,
            "out": 'F' if s == "qF" else '',
            "transitions": transitions[s]
            }
        nfa["states_with_transitions"].append(state)
    
    return nfa

def write_machine_to_csv(moore_machine, file_name):
    try:
        with open(file_name, 'w', encoding='utf-8') as file:
            for state in moore_machine["states_with_transitions"]:
                file.write(';' + state["out"])
            file.write('\n')
            for state in moore_machine["states_with_transitions"]:
                file.write(';' + state["current_state"])
            file.write('\n')
            for entry in moore_machine["entries"]:
                file.write(entry + ';')
                for state in moore_machine["states_with_transitions"]:
                    file_transits = []
                    for transit in state["transitions"]:
                        if transit["entry"] == entry:
                            file_transits = transit["states"]
                    for t in file_transits:
                        if t != file_transits[len(file_transits) - 1]:
                            file.write(t + ',')
                        else:
                            file.write(t)
                    if state != moore_machine["states_with_transitions"][len(moore_machine["states_with_transitions"]) - 1]:
                        file.write(';')
                file.write('\n')
    except FileNotFoundError:
        print("Файл не найден")
        sys.exit(1)
    except Exception as e:
        print(f"Произошла ошибка: {e}")
        sys.exit(1)

def process_regex(output_file_name, regex):
    parser = Parser(regex)
    tree = parser.parse()

    start, finite = Graph("q0"), Graph("qF")
    alphabet = []
    counter = 0

    build_nfa_from_tree(tree, start, finite, alphabet, counter)

    nfa = build_table_nfa(start, finite, alphabet)

    write_machine_to_csv(nfa, output_file_name)

def main():
    if len(sys.argv) != 3:
        print("Usage: ", sys.argv[0], " <output.csv> <regular_expression>")
        sys.exit(1)

    output_file_name = sys.argv[1]
    regex = sys.argv[2]

    process_regex(output_file_name, regex)
    sys.exit(0)

if __name__ == "__main__":
    main()

