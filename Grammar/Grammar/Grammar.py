from ast import List
import sys
import re

def read_text_from_file(file_name):
    try:
        with open(file_name, 'r', encoding='utf-8') as file:
            return file.read().strip()
    except FileNotFoundError:
        print("Файл не найден")
        sys.exit(1)
    except Exception as e:
        print(f"Произошла ошибка: {e}")
        sys.exit(1)


def write_moore_machine_to_csv(moore_machine, file_name):
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
                            file_transits.append(transit["state"])
                    for t in file_transits:
                        if t != file_transits[len(file_transits) - 1]:
                            file.write(t + ',')
                        else:
                            file.write(t)
                    if state != moore_machine["states_with_transitions"][len(moore_machine["states_with_transitions"]) - 1]["current_state"]:
                        file.write(';')
                file.write('\n')
    except FileNotFoundError:
        print("Файл не найден")
        sys.exit(1)
    except Exception as e:
        print(f"Произошла ошибка: {e}")
        sys.exit(1)


def process_left_hand_grammar(gram):
    moore_machine = {
        "entries": [],
        "states_with_transitions": []
    }

    list_of_grammars = re.findall(r'^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?[\wε](?:\s*\|\s*(?:<\w+>\s+)?[\wε])*)\s*$', gram, flags=re.MULTILINE)
    for pairs in list_of_grammars:
        moore_machine["states_with_transitions"].append({ "current_state": pairs[0], "out": "", "transitions": [] })
    moore_machine["states_with_transitions"].append({ "current_state": "H", "out": "", "transitions": [] })

    transits = []
    for pairs in list_of_grammars:
        transits_for_one_state = [t.strip() for t in pairs[1].split('|')]
        transits.append(transits_for_one_state)

    for i in range(len(transits)):
        for t in transits[i]:
            match = re.search(r'\s*<(\w+)>\s+([\wε])\s*', t, flags=re.MULTILINE)

            if match:
                if match[2] not in moore_machine["entries"]:
                    moore_machine["entries"].append(match[2])
                for state in moore_machine["states_with_transitions"]:
                    if state["current_state"] == match[1]:
                        state["transitions"].append({"state": moore_machine["states_with_transitions"][i]["current_state"], "entry": match[2]})

            else:
                match = re.search(r'\s*([\wε])\s*', t, flags=re.MULTILINE)
                if match:
                    if match[1] not in moore_machine["entries"]:
                        moore_machine["entries"].append(match[1])
                    moore_machine["states_with_transitions"][len(moore_machine["states_with_transitions"]) - 1]["transitions"].append({"state": moore_machine["states_with_transitions"][i]["current_state"], "entry": match[1]})
                else:
                    print("Cannot interpret grammar")
                    sys.exit(1)

    for state in moore_machine["states_with_transitions"]:
        if not state["transitions"]:
            state["out"] = 'F'

    return moore_machine


def process_right_hand_grammar(gram):
    moore_machine = {
        "entries": [],
        "states_with_transitions": []
    }

    list_of_grammars = re.findall(r'^\s*<(\w+)>\s*->\s*([\wε](?:\s+<\w+>)?(?:\s*\|\s*[\wε](?:\s+<\w+>)?)*)\s*$', gram, flags=re.MULTILINE)

    for pairs in list_of_grammars:
        moore_machine["states_with_transitions"].append({ "current_state": pairs[0], "out": "", "transitions": [] })
    moore_machine["states_with_transitions"].append({ "current_state": "F", "out": "F", "transitions": [] })

    transits = []
    for pairs in list_of_grammars:
        transits_for_one_state = [t.strip() for t in pairs[1].split('|')]
        transits.append(transits_for_one_state)

    for i in range(len(transits)):
        for t in transits[i]:
            match = re.search(r'\s*([\wε])\s+<(\w+)>\s*', t, flags=re.MULTILINE)

            if match:
                if match[1] not in moore_machine["entries"]:
                    moore_machine["entries"].append(match[1])
                moore_machine["states_with_transitions"][i]["transitions"].append({"state": match[2], "entry": match[1]})

            else:
                match = re.search(r'\s*([\wε])\s*', t, flags=re.MULTILINE)
                if match:
                    if match[1] not in moore_machine["entries"]:
                        moore_machine["entries"].append(match[1])
                    moore_machine["states_with_transitions"][i]["transitions"].append({"state": "F", "entry": match[1]})
                else:
                    print("Cannot interpret grammar")
                    sys.exit(1)

    return moore_machine


def process_grammar(in_file_name, out_file_name):
    grammar = read_text_from_file(in_file_name)
    
    if len(re.findall(r'^\s*<(\w+)>\s*->\s*([\wε](?:\s+<\w+>)?(?:\s*\|\s*[\wε](?:\s+<\w+>)?)*)\s*$', grammar, flags=re.MULTILINE)) == grammar.count('->'):
        moore_machine = process_right_hand_grammar(grammar)
    elif len(re.findall(r'^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?[\wε](?:\s*\|\s*(?:<\w+>\s+)?[\wε])*)\s*$', grammar, flags=re.MULTILINE)) == grammar.count('->'):
        moore_machine = process_left_hand_grammar(grammar)
    else:
        print("wrong grammar was read")
        sys.exit(1)

    write_moore_machine_to_csv(moore_machine, out_file_name)

def main():
    if len(sys.argv) != 3:
        print("Usage: ", sys.argv[0], " <input.txt> <output.csv>")
        sys.exit(1)

    input_file_name = sys.argv[1]
    output_file_name = sys.argv[2]

    process_grammar(input_file_name, output_file_name)


if __name__ == "__main__":
    main()
